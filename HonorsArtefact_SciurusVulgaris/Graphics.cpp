#include "PCH.h"
#include "Graphics.h"

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

    VkRenderPassBeginInfo renderPassInfo{};
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
    vkCmdSetScissor(instance.vkCommandBuffers[instance.currentFrame], 0, 1, &scissor);

    instance.thisFramesDrawCall = 0;
}

void Graphics::Render(PointMesh* points)
{
    VkBuffer vertexBuffers[] = { points->pointBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(instance.vkCommandBuffers[instance.currentFrame], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(instance.vkCommandBuffers[instance.currentFrame], points->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    if (instance.thisFramesDrawCall >= instance.perFramePerObjectDescriptors[instance.currentFrame].size())
        AddAdditionalDescriptorSet(instance.perFramePerObjectDescriptors, instance.vkDescriptorSetLayout);

    instance.frameinc++;
    WCP_Matrices dataForUBO{
        HMM_Rotate_LH(instance.frameinc / 1000.0f, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -10), HMM_V3(0, 0, 0), HMM_V3(0, 1, 0)), HMM_Perspective_LH_ZO(140, 1, 0.001, 30)
    };

    memcpy(instance.perFramePerObjectDescriptors[instance.currentFrame][instance.thisFramesDrawCall].GetMappedMemoryLocation(0), &dataForUBO, sizeof(WCP_Matrices));

    vkCmdBindDescriptorSets(instance.vkCommandBuffers[instance.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, instance.vkMainPipelineLayout, 0, 1,
        instance.perFramePerObjectDescriptors[instance.currentFrame][instance.thisFramesDrawCall].GetDescriptorSet(), 0, nullptr);

    vkCmdDrawIndexed(instance.vkCommandBuffers[instance.currentFrame], static_cast<uint32_t>(points->indices.size()), 1, 0, 0, 0);

    instance.thisFramesDrawCall++;
}

void Graphics::EndRender()
{
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), instance.vkCommandBuffers[instance.currentFrame]);
    // Finish recording command buffer
    vkCmdEndRenderPass(instance.vkCommandBuffers[instance.currentFrame]);

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

    VkSemaphore signalSemaphores[] = { instance.vkRenderFinishedSemaphores[instance.currentFrame] };
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

    /*if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || swapChainNeedsRecreation) {
        swapChainNeedsRecreation = false;
        if (!Services::GetTree()->IsGameClosingThisFrame())RecreateSwapChain();
    }
    else if (result != VK_SUCCESS) {
        throw - 1;
    }*/

    instance.currentFrame = (instance.currentFrame + 1) % VULKAN_MAX_FRAMES_IN_FLIGHT;
}

void Graphics::AddAdditionalDescriptorSet(std::vector<std::vector<VulkanDescriptor>>& descriptorSetList, const VkDescriptorSetLayout& setLayout)
{
    std::vector<size_t> sizes = { sizeof(WCP_Matrices) };

    for (int i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
        int newSetIndex = descriptorSetList[i].size();
        descriptorSetList[i].push_back(VulkanDescriptor());
        descriptorSetList[i][newSetIndex].CreateAndAllocateBuffers(sizes.data(), sizes.size());
        descriptorSetList[i][newSetIndex].CreateDescriptorSet(instance.vkDevice, instance.vkDescriptorSetLayout, instance.vkDescriptorPool);
    }
}