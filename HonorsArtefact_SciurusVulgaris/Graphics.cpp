#include "PCH.h"
#include "Graphics.h"
#include "VulkanSetup.h"
#include "Input.h"

Graphics Graphics::instance;

void Graphics::BeginRender()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Wait until previous frame is finished. 
    vkWaitForFences(instance.vkDevice, 1, &instance.vkInFlightFences[instance.currentFrame], VK_TRUE, UINT64_MAX);

    // Get image from swap chain
    VkResult result = vkAcquireNextImageKHR(instance.vkDevice, instance.vkSwapChain, UINT64_MAX,
        instance.vkImageAvailableSemaphores[instance.currentFrame], VK_NULL_HANDLE, &instance.thisRenderImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        //RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw - 1;
    }

    // Reset it only if we know we can draw
    vkResetFences(instance.vkDevice, 1, &instance.vkInFlightFences[instance.currentFrame]);

    // Reset command buffer
    vkResetCommandBuffer(instance.vkCommandBuffers[instance.currentFrame], 0);

    // Record command buffer setup
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(instance.vkCommandBuffers[instance.currentFrame], &beginInfo) != VK_SUCCESS) {
        throw - 1;
    }
    // Do mesh render
    /*instance.meshRenderer.BeginRender(HMM_V4(0.3f, 0.6f, 0.8f, 1.0f));
    instance.meshRenderer.Render(instance.myMesh);
    instance.meshRenderer.EndRender();*/

    /*VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = instance.vkRenderPass;
    renderPassInfo.framebuffer = instance.vkSwapChainFrameBuffers[instance.thisRenderImageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = instance.vkSwapChainExtent;

    std::vector<VkClearValue> clearColors = { {{instance.clearColor.R, instance.clearColor.G, instance.clearColor.B, instance.clearColor.A}}, {1.0f, 0} };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(instance.vkCommandBuffers[instance.currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(instance.vkCommandBuffers[instance.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, instance.vkMainPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(instance.vkSwapChainExtent.width);
    viewport.height = static_cast<float>(instance.vkSwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(instance.vkCommandBuffers[instance.currentFrame], 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = instance.vkSwapChainExtent;
    vkCmdSetScissor(instance.vkCommandBuffers[instance.currentFrame], 0, 1, &scissor);*/

    instance.thisFramesDrawCall = 0;
}

void Graphics::FinishImGuiRender()
{
    ImGui::ShowDemoWindow();

    /*ImGui::Begin("Mesh");
    ImGui::Image(instance.meshRenderOutput, ImVec2(300, 300));
    ImGui::Image(instance.meshRenderOutput2, ImVec2(300, 300));
    ImGui::Image(instance.meshRenderOutput3, ImVec2(300, 300));
    ImGui::End();*/

    instance.VRAMAllocator.RenderMemoryUsageStat();

    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), instance.vkCommandBuffers[instance.currentFrame]);
}

void Graphics::EndRender()
{
    //ImGui::ShowDemoWindow();

    //ImGui::Begin("Mesh");
    //ImGui::Image(instance.meshRenderOutput, ImVec2(300, 300));
    //ImGui::Image(instance.meshRenderOutput2, ImVec2(300, 300));
    //ImGui::Image(instance.meshRenderOutput3, ImVec2(300, 300));
    //ImGui::End();

    //instance.VRAMAllocator.RenderMemoryUsageStat();

    //ImGui::Render();
    //ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), instance.vkCommandBuffers[instance.currentFrame]);
    //// Finish recording command buffer
    //vkCmdEndRenderPass(instance.vkCommandBuffers[instance.currentFrame]);

    if (vkEndCommandBuffer(instance.vkCommandBuffers[instance.currentFrame]) != VK_SUCCESS) {
        throw - 1;
    }

    // Now we need to submit it
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { instance.vkImageAvailableSemaphores[instance.currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // We only care about colour writing, this allows pre rasteriser to get head start
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &instance.vkCommandBuffers[instance.currentFrame];

    VkSemaphore signalSemaphores[] = { instance.vkRenderFinishedSemaphores[instance.thisRenderImageIndex] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(instance.vkGraphicsQueue, 1, &submitInfo, instance.vkInFlightFences[instance.currentFrame]) != VK_SUCCESS) {
        throw - 1;
    }

    //std::cout << "Draws this frame: " << thisFramesDrawCall << "\n";

    // Now we present
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { instance.vkSwapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &instance.thisRenderImageIndex;

    presentInfo.pResults = nullptr; // Optional

    VkResult result = vkQueuePresentKHR(instance.vkPresentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || instance.swapChainNeedsRecreation) {
        instance.swapChainNeedsRecreation = false;
        /*if (!Services::GetTree()->IsGameClosingThisFrame())*/ RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw - 1;
    }

    instance.currentFrame = (instance.currentFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
}

void Graphics::RegisterWindowSizeChange(HMM_Vec2 newSize)
{
    instance.swapChainNeedsRecreation = true;
    instance.currentWidth = static_cast<int>(newSize.X);
    instance.currentHeight = static_cast<int>(newSize.Y);
}

HMM_Vec2 Graphics::GetWindowLocation()
{
    RECT rect = { NULL };
    HMM_Vec2 location = HMM_V2(0, 0);
    if (GetWindowRect(instance.window, &rect)) {
        location.X = rect.left;
        location.Y = rect.top;
    }

    return location;
}

void Graphics::RecreateSwapChain()
{
    // If we are minimised, size is 0, freeze all main thread processing appart from the input loop (as this will see when we unminimise)
    while (instance.currentWidth == 0 || instance.currentHeight == 0) {
        Input::ProcessEvents();
    }
    // Wait until nothing is going on
    vkDeviceWaitIdle(instance.vkDevice);

    // Destroy Frame Buffers
    for (auto& thisFrameBuffer : instance.vkSwapChainFrameBuffers) vkDestroyFramebuffer(instance.vkDevice, thisFrameBuffer, nullptr);

    // Destroy Images & Swap Chain
    for (auto& thisImageView : instance.vkSwapChainImageViews) vkDestroyImageView(instance.vkDevice, thisImageView, nullptr);
    vkDestroySwapchainKHR(instance.vkDevice, instance.vkSwapChain, nullptr);
    vkDestroyImageView(instance.vkDevice, instance.vkDepthImageView, nullptr);
    vkDestroyImage(instance.vkDevice, instance.vkDepthImage, nullptr);
    vkFreeMemory(instance.vkDevice, instance.vkDepthImageMemory, nullptr);

    VulkanSetup::CreateSwapChain(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSurface,
        instance.currentWidth, instance.currentHeight,
        &instance.vkSwapChainFormat, &instance.vkSwapChainExtent, &instance.vkSwapChainImages, &instance.vkSwapChain);

    VulkanSetup::CreateImageViewsForSwapChain(instance.vkDevice, instance.vkSwapChainFormat,
        instance.vkSwapChainImages, &instance.vkSwapChainImageViews);

    VulkanSetup::CreateDepthBuffer(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSwapChainExtent, &instance.vkDepthImage,
        &instance.vkDepthImageMemory, &instance.vkDepthImageView);

    // Setup frame buffers
    VulkanSetup::CreateFrameBuffers(instance.vkDevice, instance.vkRenderPass, instance.vkSwapChainExtent, instance.vkSwapChainImageViews, instance.vkDepthImageView, &instance.vkSwapChainFrameBuffers);


}

//void Graphics::AddAdditionalDescriptorSet(std::vector<std::vector<VulkanObjectDescriptorSet>>& descriptorSetList, const VkDescriptorSetLayout& setLayout)
//{
//    std::vector<size_t> sizes = { sizeof(WCP_Matrices) };
//
//    for (int i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
//        int newSetIndex = descriptorSetList[i].size();
//        descriptorSetList[i].push_back(VulkanObjectDescriptorSet());
//        descriptorSetList[i][newSetIndex].CreateAndAllocateBuffers(sizes.data(), sizes.size());
//        descriptorSetList[i][newSetIndex].CreateDescriptorSet(instance.vkDevice, instance.vkDescriptorSetLayout, instance.vkDescriptorPool);
//    }
//}