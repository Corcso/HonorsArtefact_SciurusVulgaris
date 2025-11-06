#include "PCH.h"
#include "MeshRenderer.h"
#include "Graphics.h"
#include "VulkanSetup.h"
#include "VulkanUtility.h"

void MeshRenderer::Shutdown()
{
    // Destroy Pipeline
    vkDestroyPipeline(Graphics::GetVkDevice(), vkMainPipeline, nullptr);
    vkDestroyPipelineLayout(Graphics::GetVkDevice(), vkMainPipelineLayout, nullptr);

    // Destroy Frame Buffer
    vkDestroyFramebuffer(Graphics::GetVkDevice(), vkFrameBuffer, nullptr);

    // Destroy Render Pass
    vkDestroyRenderPass(Graphics::GetVkDevice(), vkRenderPass, nullptr);

    // Destroy Sampler
    vkDestroySampler(Graphics::GetVkDevice(), vkSampler, nullptr);

    // Destroy Images & Swap Chain
    Graphics::GetMemoryAllocator().FreeMemory(Graphics::GetVkDevice(), vkColorImageMemory);
    vkDestroyImageView(Graphics::GetVkDevice(), vkColorImageView, nullptr);
    vkDestroyImage(Graphics::GetVkDevice(), vkColorImage, nullptr);
    Graphics::GetMemoryAllocator().FreeMemory(Graphics::GetVkDevice(), vkDepthImageMemory);
    vkDestroyImageView(Graphics::GetVkDevice(), vkDepthImageView, nullptr);
    vkDestroyImage(Graphics::GetVkDevice(), vkDepthImage, nullptr);

    // Destroy descriptors
    for (auto& descriptor : perObjectDescriptors) {
        descriptor.CleanupDescriptor();
    }
    
    perObjectDescriptors.clear();
    vkDestroyDescriptorSetLayout(Graphics::GetVkDevice(), vkDescriptorSetLayout, nullptr);
}

void MeshRenderer::CreateImages()
{
    // Create local image for color
    vkColorImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
    vkColorImageExtent.width = 512;
    vkColorImageExtent.height = 512;


    VulkanUtility::CreateImageAndAssignMemory(vkColorImageExtent.width, vkColorImageExtent.height, vkColorImageFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkColorImage, &vkColorImageMemory);

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = vkColorImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vkColorImageFormat;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(Graphics::GetVkDevice(), &imageViewCreateInfo, nullptr, &vkColorImageView);

    // Same for depth
    VulkanUtility::CreateImageAndAssignMemory(vkColorImageExtent.width, vkColorImageExtent.height, VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()),
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkDepthImage, &vkDepthImageMemory);

    // Make tweaks
    imageViewCreateInfo.image = vkDepthImage;
    imageViewCreateInfo.format = VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice());
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

    vkCreateImageView(Graphics::GetVkDevice(), &imageViewCreateInfo, nullptr, &vkDepthImageView);


    // Attempt position
    vkPositionImageFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
    vkPositionImageExtent.width = 512;
    vkPositionImageExtent.height = 512;


    VulkanUtility::CreateImageAndAssignMemory(vkPositionImageExtent.width, vkPositionImageExtent.height, vkPositionImageFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkPositionImage, &vkPositionImageMemory);

    imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = vkPositionImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vkPositionImageFormat;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(Graphics::GetVkDevice(), &imageViewCreateInfo, nullptr, &vkPositionImageView);
}

void MeshRenderer::CreateSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
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

void MeshRenderer::CreateRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = vkColorImageFormat;
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
    depthAttachmentRef.attachment = 2;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Position buffer attachment image
    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = vkPositionImageFormat;
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

    VkAttachmentReference colorAttachments[2]{colorAttachmentRef, positionAttachmentRef};

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 2;
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
    std::vector<VkAttachmentDescription> attachments = { colorAttachment, positionAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(Graphics::GetVkDevice() , &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw - 1;
    }
}

void MeshRenderer::CreateFrameBuffer()
{
    VkImageView imageViewList[]{ vkColorImageView,vkPositionImageView, vkDepthImageView  };

    VkFramebufferCreateInfo frameBufferCreateInfo{};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.width = vkColorImageExtent.width;
    frameBufferCreateInfo.height = vkColorImageExtent.height;
    frameBufferCreateInfo.attachmentCount = 3;
    frameBufferCreateInfo.pAttachments = imageViewList;
    frameBufferCreateInfo.renderPass = vkRenderPass;
    frameBufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(Graphics::GetVkDevice(), &frameBufferCreateInfo, nullptr, &vkFrameBuffer) != VK_SUCCESS) {
        throw - 1;
    }
}

void MeshRenderer::CreateDescriptorLayout()
{
    std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings(1);
    uboLayoutBindings[0].binding = 0;
    uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBindings[0].descriptorCount = 1;
    uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBindings[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uboLayoutBindings.size();
    layoutInfo.pBindings = uboLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(Graphics::GetVkDevice(), &layoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
        throw - 1;
    }
}

void MeshRenderer::CreatePipeline()
{
    // Get shader code

    auto vertShaderCode = VulkanUtility::ReadFile("./VULKAN_COMPILED_vertex.spv");
    auto fragShaderCode = VulkanUtility::ReadFile("./COMPILEDSHADER_MeshFragment.spv");

    VkShaderModule vertShaderModule = VulkanUtility::CreateShaderModule(Graphics::GetVkDevice(), vertShaderCode);
    VkShaderModule fragShaderModule = VulkanUtility::CreateShaderModule(Graphics::GetVkDevice(), fragShaderCode);

    // Vertex shader
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // Fragment shader
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    // Array these
    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

    // Some things can be dynamic, viewport should be for ease
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Vertex imput  stage setup // TODO when can i dealloc this
    VkVertexInputAttributeDescription* vertexAttributeDescriptions = new VkVertexInputAttributeDescription[2];
    uint32_t vertexAttributeDescriptionCount = 2;

    vertexAttributeDescriptions[0].binding = 0;
    vertexAttributeDescriptions[0].location = 0;
    vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[0].offset = offsetof(PointMesh::Point, position);

    vertexAttributeDescriptions[1].binding = 0;
    vertexAttributeDescriptions[1].location = 1;
    vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[1].offset = offsetof(PointMesh::Point, color);

    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.binding = 0;
    vertexBindingDescription.stride = sizeof(PointMesh::Point);
    vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkVertexInputBindingDescription bindingDesc = vertexBindingDescription;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions;

    // Setup input assembler
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport see section in https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vkColorImageExtent.width;
    viewport.height = (float)vkColorImageExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // Scissor rectangle is the area which isnt discarded by the rasterizer, we want whole viewport for now
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = vkColorImageExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Depth Stencil State
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // Optional functionality to only care about depth values in a range (we dont care)
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    // No stencil testing
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Multisampling, disabled
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Colour blending (blending section of the OM)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachments[2]{ colorBlendAttachment, colorBlendAttachment };

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 2;
    colorBlending.pAttachments = colorBlendAttachments;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    // Pipeline layout, this is like your constant buffer setup bit
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // 
    pipelineLayoutInfo.pSetLayouts = &vkDescriptorSetLayout; // 
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(Graphics::GetVkDevice(), &pipelineLayoutInfo, nullptr, &vkMainPipelineLayout) != VK_SUCCESS) {
        throw - 1;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = vkMainPipelineLayout;

    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(Graphics::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkMainPipeline) != VK_SUCCESS) {
        throw - 1;
    }

    vkDestroyShaderModule(Graphics::GetVkDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(Graphics::GetVkDevice(), vertShaderModule, nullptr);
}

void MeshRenderer::BeginRender(HMM_Vec4 clearColor)
{
    // Assume command buffer is ready and open
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFrameBuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = vkColorImageExtent;

    std::vector<VkClearValue> clearColors = { {{clearColor.R, clearColor.G, clearColor.B, clearColor.A}}, {{0, 0, 0, 0}}, {1.0f, 0}};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(Graphics::GetThisFramesCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(Graphics::GetThisFramesCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vkMainPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(vkColorImageExtent.width);
    viewport.height = static_cast<float>(vkColorImageExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(Graphics::GetThisFramesCommandBuffer(), 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = vkColorImageExtent;
    vkCmdSetScissor(Graphics::GetThisFramesCommandBuffer(), 0, 1, &scissor);

    thisFramesDrawCall = 0;
}

void MeshRenderer::Render(TriListMesh* mesh)
{
    VkBuffer vertexBuffers[] = { mesh->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(Graphics::GetThisFramesCommandBuffer(), 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(Graphics::GetThisFramesCommandBuffer(), mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // TODO MAKE descriptors better
    if (thisFramesDrawCall >= perObjectDescriptors.size())
    {
        std::vector<size_t> sizes = { sizeof(WCP_Matrices) };

        int newSetIndex = perObjectDescriptors.size();
        perObjectDescriptors.push_back(VulkanDescriptor());
        perObjectDescriptors[newSetIndex].CreateAndAllocateBuffers(sizes.data(), sizes.size());
        perObjectDescriptors[newSetIndex].CreateDescriptorSet(Graphics::GetVkDevice(), vkDescriptorSetLayout, Graphics::GetDescriptorPool()); // Should i use the same one
    }

    frameinc++;
    WCP_Matrices dataForUBO{
        HMM_Rotate_LH(frameinc / 1000.0f, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -10), HMM_V3(0, 0, 0), HMM_V3(0, 1, 0)), HMM_Perspective_LH_ZO(50, 1, 0.001, 30)
    };

    memcpy(perObjectDescriptors[thisFramesDrawCall].GetMappedMemoryLocation(0), &dataForUBO, sizeof(WCP_Matrices));

    vkCmdBindDescriptorSets(Graphics::GetThisFramesCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, vkMainPipelineLayout, 0, 1,
        perObjectDescriptors[thisFramesDrawCall].GetDescriptorSet(), 0, nullptr);

    vkCmdDrawIndexed(Graphics::GetThisFramesCommandBuffer(), static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);

    thisFramesDrawCall++;
}

void MeshRenderer::EndRender()
{
    vkCmdEndRenderPass(Graphics::GetThisFramesCommandBuffer());

    // I think I can get away with the below as I instruct the render pass to finish with the attachment in shader state

    //// Wait for viewport to be available for rendering
    //// TODO learn more about this!
    //VkImageMemoryBarrier barrier{};
    //barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.image = vkColorImage;
    //barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //barrier.subresourceRange.baseMipLevel = 0;
    //barrier.subresourceRange.levelCount = 1;
    //barrier.subresourceRange.baseArrayLayer = 0;
    //barrier.subresourceRange.layerCount = 1;
    //barrier.srcAccessMask = 0; // TODO
    //barrier.dstAccessMask = 0; // TODO

    //vkCmdPipelineBarrier(
    //    Graphics::GetThisFramesCommandBuffer(),
    //    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT /* TODO */, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT /* TODO */,
    //    0,
    //    0, nullptr,
    //    0, nullptr,
    //    1, &barrier
    //);
}
