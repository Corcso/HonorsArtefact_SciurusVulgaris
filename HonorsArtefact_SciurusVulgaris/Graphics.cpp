#include "PCH.h"
#include "Graphics.h"
#include "VulkanSetup.h"
#include "VulkanUtility.h"
#include "Input.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

Graphics Graphics::instance;

void Graphics::BeginRender()
{
#ifdef NV_PERF_METER
    VkResult res = vkDeviceWaitIdle(instance.vkDevice);
    if (res != VK_SUCCESS){
        std::cout << "CANNOT IDLE WAIT FOR NVPERF\n";
    }
    vkQueueWaitIdle(instance.vkGraphicsQueue);
    vkQueueWaitIdle(instance.vkPresentQueue);

    if (instance.nvperf_InitiateReportNextFrame && !instance.nvperf_liveMode) {
        instance.nvperf_reportGenerator.StartCollectionOnNextFrame();
        instance.nvperf_InitiateReportNextFrame = false;
    }
    instance.nvperf_reportGenerator.OnFrameStart(instance.vkPresentQueue, VulkanSetup::GetQueueFamilyIndices(instance.vkPhysicalDevice, instance.vkSurface).presentFamily);
#endif
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
    PushMetricRange("Program Render");
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
    PopMetricRange(); // Program Render

    //ImGui::ShowDemoWindow();

    /*ImGui::Begin("Mesh");
    ImGui::Image(instance.meshRenderOutput, ImVec2(300, 300));
    ImGui::Image(instance.meshRenderOutput2, ImVec2(300, 300));
    ImGui::Image(instance.meshRenderOutput3, ImVec2(300, 300));
    ImGui::End();*/

    //instance.VRAMAllocator.RenderMemoryUsageStat();

#ifdef NV_PERF_METER
    if (instance.nvperf_liveMode) {
        instance.nvperf_sampler.DecodeCounters();
        instance.nvperf_sampler.ConsumeSamples([&](const uint8_t* pCounterDataImage, size_t counterDataImageSize, uint32_t rangeIndex, bool& stop) {
            stop = false;
            return instance.nvperf_hudDataModel.AddSample(pCounterDataImage, counterDataImageSize, rangeIndex);
            });
        for (auto& frameDelimiter : instance.nvperf_sampler.GetFrameDelimiters())
        {
            instance.nvperf_hudDataModel.AddFrameDelimiter(frameDelimiter.frameEndTime);
        }

        ImGui::SetNextWindowSize(ImVec2(400, -1), ImGuiCond_Appearing);
        ImGui::Begin("Graphics General Triage");
        instance.nvperf_hudRenderer.Render();
        ImGui::End();
    }
#endif // NV_PERF_METER


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

#ifdef NV_PERF_METER
    
    ////instance.nvperf_reportGenerator.Reset();
    //vkQueueWaitIdle(instance.vkGraphicsQueue);
    //vkQueueWaitIdle(instance.vkPresentQueue);
    vkDeviceWaitIdle(instance.vkDevice);
    VkResult result = vkQueuePresentKHR(instance.vkPresentQueue, &presentInfo);
    instance.nvperf_reportGenerator.OnFrameEnd();
    if(instance.nvperf_liveMode) instance.nvperf_sampler.OnFrameEnd();
    //if (instance.nvperf_InitiateReportNextFrame) {
    //    instance.nvperf_reportGenerator.StartCollectionOnNextFrame();
    //    instance.nvperf_InitiateReportNextFrame = false;
    //}
    //instance.nvperf_reportGenerator.OnFrameStart(instance.vkGraphicsQueue, VulkanSetup::GetQueueFamilyIndices(instance.vkPhysicalDevice, instance.vkSurface).graphicsFamily);
#else
    VkResult result = vkQueuePresentKHR(instance.vkPresentQueue, &presentInfo);
#endif // NV_PERF_METER

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

void Graphics::SaveSwapChainImageToFile(std::string path)
{
    // Create CPU visible transfer DST buffer
    VkBuffer stagingBuffer;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingBufferMemory;

    VulkanUtility::CreateBufferAndAssignMemory(instance.vkSwapChainExtent.width * instance.vkSwapChainExtent.height * 4 /*My magic number!*/, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    VulkanUtility::TransitionImageLayout(instance.vkSwapChainImages[0], instance.vkSwapChainFormat, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VulkanUtility::CopyImageToBuffer(stagingBuffer, instance.vkSwapChainImages[0], instance.vkSwapChainExtent.width, instance.vkSwapChainExtent.height);

    VulkanUtility::TransitionImageLayout(instance.vkSwapChainImages[0], instance.vkSwapChainFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    char* data = new char[instance.vkSwapChainExtent.width * instance.vkSwapChainExtent.height * 4];
    VulkanUtility::MapCopyBlockFromGPU(stagingBufferMemory, data, instance.vkSwapChainExtent.width * instance.vkSwapChainExtent.height * 4);

    vkDestroyBuffer(instance.vkDevice, stagingBuffer, nullptr);
    instance.VRAMAllocator.FreeMemory(instance.vkDevice, stagingBufferMemory);

    stbi_write_bmp(path.c_str(), instance.vkSwapChainExtent.width, instance.vkSwapChainExtent.height, 4, data);

    delete[] data;
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
#ifdef NV_PERF_METER
void Graphics::nvperf_InitiateReport(std::string folder)
{
    instance.nvperf_reportGenerator.outputOptions.directoryName = "nvperfout\\" + folder;
    instance.nvperf_InitiateReportNextFrame = true;
}
std::string Graphics::nfperf_GetLastReportDir()
{
    return instance.nvperf_reportGenerator.GetLastReportDirectoryName();
}
#endif // NV_PERF_METER
void Graphics::PushMetricRange(std::string name)
{
    // For ease this function can be called anytime, but does nothing if not in NVPERF mode
#ifdef NV_PERF_METER
    instance.nvperf_reportGenerator.rangeCommands.PushRange(instance.vkCommandBuffers[instance.currentFrame], name.c_str());
#endif // NV_PERF_METER
}

void Graphics::PopMetricRange()
{
    // For ease this function can be called anytime, but does nothing if not in NVPERF mode
#ifdef NV_PERF_METER
    instance.nvperf_reportGenerator.rangeCommands.PopRange(instance.vkCommandBuffers[instance.currentFrame]);
#endif // NV_PERF_METER
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