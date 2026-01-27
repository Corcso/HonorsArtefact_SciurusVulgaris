#include "PCH.h"
#include "InstancedTreeRenderPass.h"
#include "VulkanUtility.h"
#include "Graphics.h"
#include "VulkanSetup.h"
#include "Light.h"

void InstancedTreeRenderPass::CreateImages() {
    colorImage.CreateImage(VK_FORMAT_R8G8B8A8_UNORM, Graphics::GetSwapChainExtent().width, Graphics::GetSwapChainExtent().height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    colorImage.CreateImageView();

    positionImage.CreateImage(VK_FORMAT_R32G32B32A32_SFLOAT, Graphics::GetSwapChainExtent().width, Graphics::GetSwapChainExtent().height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    positionImage.CreateImageView();

    normalImage.CreateImage(VK_FORMAT_R32G32B32A32_SFLOAT, Graphics::GetSwapChainExtent().width, Graphics::GetSwapChainExtent().height, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    normalImage.CreateImageView();

    depthImage.CreateImage(VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()), Graphics::GetSwapChainExtent().width, Graphics::GetSwapChainExtent().height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthImage.CreateImageView(true);
}

void InstancedTreeRenderPass::CreateUniqueMeshData()
{
    fullScreenQuad = new TriListMesh();

    fullScreenQuad->vertices.resize(4);
                  
    fullScreenQuad->vertices[0].position = HMM_V3(-1, -1, 0);
    fullScreenQuad->vertices[0].normal = HMM_V3(0, 0, -1);
    fullScreenQuad->vertices[0].textureCoordinate = HMM_V2(0, 0);
    fullScreenQuad->vertices[1].position = HMM_V3(1, -1, 0);
    fullScreenQuad->vertices[1].normal = HMM_V3(0, 0, -1);
    fullScreenQuad->vertices[1].textureCoordinate = HMM_V2(1, 0);
    fullScreenQuad->vertices[2].position = HMM_V3(-1, 1, 0);
    fullScreenQuad->vertices[2].normal = HMM_V3(0, 0, -1);
    fullScreenQuad->vertices[2].textureCoordinate = HMM_V2(0, 1);
    fullScreenQuad->vertices[3].position = HMM_V3(1, 1, 0);
    fullScreenQuad->vertices[3].normal = HMM_V3(0, 0, -1);
    fullScreenQuad->vertices[3].textureCoordinate = HMM_V2(1, 1);
                  
    fullScreenQuad->indices = { 0, 2, 1, 2, 3, 1 };
                  
    fullScreenQuad->CopyPointsToVRAM();
    size_t sizes[] = { 0, 0, 0, sizeof(Light::BufferStruct) };
    fullScreenQuad->CreateDescriptorSet(gBufferToOutput_GP.vkDescriptorSetLayout, gBufferToOutput_GP.vkDescriptorSetLayoutInfo, sizes);
    fullScreenQuad->GetDescriptorSet()->UpdateImageSampler(0, &colorImage, vkSampler);
    fullScreenQuad->GetDescriptorSet()->UpdateImageSampler(1, &positionImage, vkSampler);
    fullScreenQuad->GetDescriptorSet()->UpdateImageSampler(2, &normalImage, vkSampler);
}

void InstancedTreeRenderPass::CreateSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(Graphics::GetVkDevice(), &samplerInfo, nullptr, &vkSampler) != VK_SUCCESS) {
        throw - 1;
    }
}

void InstancedTreeRenderPass::CreateFrameBuffer() {
    VkImageView imageViewList[]{ colorImage.GetImageView(), positionImage.GetImageView(), normalImage.GetImageView(), depthImage.GetImageView() };

    VkFramebufferCreateInfo frameBufferCreateInfo{};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.width = colorImage.GetImageExtent().width;
    frameBufferCreateInfo.height = colorImage.GetImageExtent().height;
    frameBufferCreateInfo.attachmentCount = 4;
    frameBufferCreateInfo.pAttachments = imageViewList;
    frameBufferCreateInfo.renderPass = vkRenderPass;
    frameBufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(Graphics::GetVkDevice(), &frameBufferCreateInfo, nullptr, &vkFrameBuffer) != VK_SUCCESS) {
        throw - 1;
    }
}

void InstancedTreeRenderPass::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = colorImage.GetImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // Load and store for colour and depth data.
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    // Load and store for stencil data. 
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Read tutorial its hard to explain
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Depth buffer attachment image
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice());
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 3;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Position buffer attachment image
    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = positionImage.GetImageFormat();
    positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    positionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentReference positionAttachmentRef{};
    positionAttachmentRef.attachment = 1;
    positionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Normal buffer attachment image
    VkAttachmentDescription normalAttachment{};
    normalAttachment.format = normalImage.GetImageFormat();
    normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAttachmentReference normalAttachmentRef{};
    normalAttachmentRef.attachment = 2;
    normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[3]{ colorAttachmentRef, positionAttachmentRef, normalAttachmentRef };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 3;
    subpass.pColorAttachments = colorAttachments;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Subpass dependencies (not sure what these are at all)
    // https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Create render pass
    std::vector<VkAttachmentDescription> attachments = { colorAttachment, positionAttachment,normalAttachment, depthAttachment };
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

void InstancedTreeRenderPass::BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFrameBuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = colorImage.GetImageExtent();

    std::vector<VkClearValue> clearColors = { {{0, 0, 0, 0}}, {{0, 0, 0, 0}}, {{0, 0, 0, 0}}, {1.0f, 0} };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsToGBufferMeshShade_GP.vkPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(colorImage.GetImageExtent().width);
    viewport.height = static_cast<float>(colorImage.GetImageExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = colorImage.GetImageExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void InstancedTreeRenderPass::RenderPointTree(PointTreeMesh* points, uint32_t pointCountOverride, VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkBuffer vertexBuffers[] = { points->pointBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    //vkCmdBindIndexBuffer(commandBuffer, points->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    //WCP_Matrices dataForUBO{
    //    HMM_Scale(HMM_V3(5, 5, 5)) * HMM_Translate(HMM_V3(0, -0.8, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -10), HMM_V3(0, 0, -20), HMM_V3(0, -1, 0)), HMM_Perspective_RH_ZO(70, 1, 0.001, 30)
    //};

    //points->GetDescriptorSet()->UpdateUniformBufferData(0, &dataForUBO);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsToGBuffer_GP.vkPipelineLayout, 0, 1,
        points->GetDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    vkCmdDraw(commandBuffer, HMM_MIN(pointCountOverride, points->points.size()), 4000, 0, 0);
}

void InstancedTreeRenderPass::RenderPointTreeViaMeshShader(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    //vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pointsToGBufferMeshShade_GP.vkPipelineLayout, 0, 1,
        points->GetDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    //reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(Graphics::GetVkDevice(), "vkCmdDrawMeshTasksEXT"))(commandBuffer, points->GetMeshletCount() * instancingInfo.numberOfInstances, 1, 1);
    reinterpret_cast<PFN_vkCmdDrawMeshTasksEXT>(vkGetDeviceProcAddr(Graphics::GetVkDevice(), "vkCmdDrawMeshTasksEXT"))(commandBuffer, ceil(instancingInfo.numberOfInstances), 1, 1);
}

void InstancedTreeRenderPass::SwitchToTraditionalMeshPipeline(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshTraditionalToGBuffer_GP.vkPipeline);
}

void InstancedTreeRenderPass::RenderTraditionalMesh(TriListMesh* mesh, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkBuffer vertexBuffers[] = { mesh->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, meshTraditionalToGBuffer_GP.vkPipelineLayout, 0, 1,
        mesh->GetDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, mesh->indices.size(), 1, 0, 0, 0);
}

void InstancedTreeRenderPass::EndRender(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);
}

void InstancedTreeRenderPass::ExecuteSecondRender(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = Graphics::GetSwapChainRenderPass();
    renderPassInfo.framebuffer = Graphics::GetThisFramesFrameBuffer();
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = Graphics::GetSwapChainExtent();

    std::vector<VkClearValue> clearColors = { {{0, 0, 0, 1}}, {1.0f, 0} };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferToOutput_GP.vkPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(Graphics::GetSwapChainExtent().width);
    viewport.height = static_cast<float>(Graphics::GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = Graphics::GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkBuffer vertexBuffers[] = { fullScreenQuad->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, fullScreenQuad->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gBufferToOutput_GP.vkPipelineLayout, 0, 1,
        fullScreenQuad->GetDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, fullScreenQuad->indices.size(), 1, 0, 0, 0);
}

void InstancedTreeRenderPass::EndSecondRender(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);
}

void InstancedTreeRenderPass::Shutdown() {
    // Destroy Pipeline
    gBufferToOutput_GP.Shutdown();
    pointsToGBuffer_GP.Shutdown();
    meshTraditionalToGBuffer_GP.Shutdown();
    pointsToGBufferMeshShade_GP.Shutdown();

    // Destroy Render Pass
    vkDestroyRenderPass(Graphics::GetVkDevice(), vkRenderPass, nullptr);

    // Destroy Frame Buffer & Images
    vkDestroyFramebuffer(Graphics::GetVkDevice(), vkFrameBuffer, nullptr);
    colorImage.Destroy();
    depthImage.Destroy();
    positionImage.Destroy();
    normalImage.Destroy();

    // Destroy Sampler
    vkDestroySampler(Graphics::GetVkDevice(), vkSampler, nullptr);

    // Destroy mesh
    delete fullScreenQuad;
}