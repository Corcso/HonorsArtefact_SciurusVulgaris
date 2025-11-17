#include "PCH.h"
#include "ImGuiBlankRenderPass.h"
#include "VulkanUtility.h"
#include "Graphics.h"

void ImGuiBlankRenderPass::Shutdown()
{

}

void ImGuiBlankRenderPass::BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = Graphics::GetSwapChainRenderPass();
    renderPassInfo.framebuffer = Graphics::GetThisFramesFrameBuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Graphics::GetSwapChainExtent();

    std::vector<VkClearValue> clearColors = { {{clearColor.R, clearColor.G, clearColor.B, clearColor.A}}, {1.0f, 0} };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void ImGuiBlankRenderPass::EndRender(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);
}
