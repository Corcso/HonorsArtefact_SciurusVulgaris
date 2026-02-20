#include "PCH.h"
#include "LightShadow_RP.h"
#include "Graphics.h"
#include "VulkanSetup.h"

void LightShadow_RP::CreateRenderPass() {
    // Depth buffer attachment image
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice());
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pColorAttachments = nullptr;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Subpass dependencies (not sure what these are at all)
    // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create render pass
    std::vector<VkAttachmentDescription> attachments = { depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Graphics::GetVkDevice(), &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw - 1;
    }
}

void LightShadow_RP::BeginRender(Light* light, VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = light->GetShadowFrameBuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = light->GetShadowImage()->GetImageExtent();

    std::vector<VkClearValue> clearColors = { {1.0f, 0} };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsToShadowMeshShade_GP.vkPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(light->GetShadowImage()->GetImageExtent().width);
    viewport.height = static_cast<float>(light->GetShadowImage()->GetImageExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = light->GetShadowImage()->GetImageExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void LightShadow_RP::RenderPointTree(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer ){
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsToShadowMeshShade_GP.vkPipelineLayout, 0, 1,
        points->GetShadowDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(Graphics::GetVkDevice(), "vkCmdDrawMeshTasksEXT"))(commandBuffer, ceil(instancingInfo.numberOfInstances), 1, 1);
}

void LightShadow_RP::EndRender(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);
}

void LightShadow_RP::Shutdown() {
    pointsToShadowMeshShade_GP.Shutdown();
    vkDestroyRenderPass(Graphics::GetVkDevice(), vkRenderPass, nullptr);
}