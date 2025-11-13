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
    colorImage.Destroy();
    positionImage.Destroy();
    normalImage.Destroy();
    depthImage.Destroy();

    vkDestroyDescriptorSetLayout(Graphics::GetVkDevice(), vkDescriptorSetLayout, nullptr);
    delete vkDescriptorSetLayoutInfo.pBindings;
    output.~PointMesh(); // Deconstruct now!
}

void MeshRenderer::CreateImages()
{
    colorImage.CreateImage(VK_FORMAT_R8G8B8A8_UNORM, 512, 512, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    colorImage.CreateImageView();

    positionImage.CreateImage(VK_FORMAT_R32G32B32A32_SFLOAT, 512, 512, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    positionImage.CreateImageView();

    normalImage.CreateImage(VK_FORMAT_R32G32B32A32_SFLOAT, 512, 512, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
    normalImage.CreateImageView();

    depthImage.CreateImage(VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()), 512, 512, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthImage.CreateImageView(true);
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

    VkAttachmentReference colorAttachments[3]{colorAttachmentRef, positionAttachmentRef, normalAttachmentRef};

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

    if (vkCreateRenderPass(Graphics::GetVkDevice() , &renderPassInfo, nullptr, &vkRenderPass) != VK_SUCCESS) {
        throw - 1;
    }
}

void MeshRenderer::CreateFrameBuffer()
{
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

void MeshRenderer::CreateDescriptorLayout()
{
    VkDescriptorSetLayoutBinding* uboLayoutBindings = new VkDescriptorSetLayoutBinding[2]; // Will be freed in shutdown
    uboLayoutBindings[0].binding = 0;
    uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBindings[0].descriptorCount = 1;
    uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBindings[0].pImmutableSamplers = nullptr;

    uboLayoutBindings[1].binding = 1;
    uboLayoutBindings[1].descriptorCount = 1;
    uboLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBindings[1].pImmutableSamplers = nullptr;
    uboLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    vkDescriptorSetLayoutInfo = {};
    vkDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutInfo.bindingCount = 2;
    vkDescriptorSetLayoutInfo.pBindings = uboLayoutBindings;

    if (vkCreateDescriptorSetLayout(Graphics::GetVkDevice(), &vkDescriptorSetLayoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
        throw - 1;
    }
}

void MeshRenderer::CreatePipeline()
{
    // Get shader code

    auto vertShaderCode = VulkanUtility::ReadFile("./COMPILEDSHADER_MeshVertex.spv");
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
    VkVertexInputAttributeDescription* vertexAttributeDescriptions = new VkVertexInputAttributeDescription[3];
    uint32_t vertexAttributeDescriptionCount = 3;

    vertexAttributeDescriptions[0].binding = 0;
    vertexAttributeDescriptions[0].location = 0;
    vertexAttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[0].offset = offsetof(TriListMesh::Vertex, position);

    vertexAttributeDescriptions[1].binding = 0;
    vertexAttributeDescriptions[1].location = 1;
    vertexAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeDescriptions[1].offset = offsetof(TriListMesh::Vertex, normal);

    vertexAttributeDescriptions[2].binding = 0;
    vertexAttributeDescriptions[2].location = 2;
    vertexAttributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertexAttributeDescriptions[2].offset = offsetof(TriListMesh::Vertex, textureCoordinate);

    VkVertexInputBindingDescription vertexBindingDescription = {};
    vertexBindingDescription.binding = 0;
    vertexBindingDescription.stride = sizeof(TriListMesh::Vertex);
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
    viewport.width = (float)colorImage.GetImageExtent().width;
    viewport.height = (float)colorImage.GetImageExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // Scissor rectangle is the area which isnt discarded by the rasterizer, we want whole viewport for now
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = colorImage.GetImageExtent();

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

    VkPipelineColorBlendAttachmentState colorBlendAttachments[3]{ colorBlendAttachment, colorBlendAttachment, colorBlendAttachment };

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 3;
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

void MeshRenderer::CreateSyncObjects()
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;

    vkCreateFence(Graphics::GetVkDevice(), &fenceInfo, nullptr, &vkIsLastExtractionFinishedFence);
}

void MeshRenderer::BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    // Assume command buffer is ready and open
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass;
    renderPassInfo.framebuffer = vkFrameBuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = colorImage.GetImageExtent();

    std::vector<VkClearValue> clearColors = { {{clearColor.R, clearColor.G, clearColor.B, clearColor.A}}, {{0, 0, 0, 0}},{{0, 0, 0, 0}}, {1.0f, 0}};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
    renderPassInfo.pClearValues = clearColors.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkMainPipeline);

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

    thisFramesDrawCall = 0;
}

void MeshRenderer::Render(TriListMesh* mesh, VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    VkBuffer vertexBuffers[] = { mesh->vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    // TODO MAKE descriptors better
    //if (thisFramesDrawCall >= perObjectDescriptors.size())
    //{
    //    std::vector<size_t> sizes = { sizeof(WCP_Matrices), 0 };

    //    int newSetIndex = perObjectDescriptors.size();
    //    perObjectDescriptors.push_back(VulkanObjectDescriptorSet());
    //    Image* imagesOnSet[2] = { nullptr, &testImage };
    //    VkSampler samplersOnSet[2] = { VK_NULL_HANDLE, vkSampler };
    //    perObjectDescriptors[newSetIndex].CreateAndAllocateBuffers(sizes.data(), sizes.size(), imagesOnSet, samplersOnSet);
    //    perObjectDescriptors[newSetIndex].CreateDescriptorSet(Graphics::GetVkDevice(), vkDescriptorSetLayout, Graphics::GetDescriptorPool()); // Should i use the same one
    //}

    frameinc++;
    //WCP_Matrices dataForUBO{
    //    HMM_M4D(1), HMM_LookAt_LH(HMM_V3(5 * sin(frameinc / 1000.0f), 0, 5 * cos(frameinc / 1000.0f)), HMM_V3(0, 0, 0), HMM_V3(0, 1, 0)), HMM_Orthographic_LH_ZO(-1.5, 1.5, 2.2, -0.2, 0.001, 10)
    //};

    //memcpy(perObjectDescriptors[thisFramesDrawCall].GetMappedMemoryLocation(0), &dataForUBO, sizeof(WCP_Matrices));

    //mesh->GetDescriptorSet()->UpdateUniformBufferData(0, &dataForUBO);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkMainPipelineLayout, 0, 1,
        mesh->GetDescriptorSet()->GetDescriptorSet(), 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);

    thisFramesDrawCall++;
}

void MeshRenderer::EndRender(VkCommandBuffer commandBuffer)
{
    if (commandBuffer == VK_NULL_HANDLE) commandBuffer = Graphics::GetThisFramesCommandBuffer();

    vkCmdEndRenderPass(commandBuffer);

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

void MeshRenderer::ExtractPoints(TriListMesh* mesh, HMM_Vec3 viewingFrom, HMM_Vec3 upDirection)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = Graphics::GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer thisExtractionCommandBuffer;

    if (vkAllocateCommandBuffers(Graphics::GetVkDevice(), &allocInfo, &thisExtractionCommandBuffer) != VK_SUCCESS) {
        throw - 1;
    }

    // Reset command buffer
    vkResetCommandBuffer(thisExtractionCommandBuffer, 0);

    // Record command buffer setup
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(thisExtractionCommandBuffer, &beginInfo) != VK_SUCCESS) {
        throw - 1;
    }

    WCP_Matrices dataForUBO{
        HMM_Rotate_LH(3.141 /2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.03, 0.03, 0.03)), HMM_LookAt_LH(viewingFrom, HMM_V3(0, 0, 0), upDirection), HMM_Orthographic_LH_ZO(-1.5, 1.5, 2.2, -0.5, 0.001, 10)
    };
    mesh->GetDescriptorSet()->UpdateUniformBufferData(0, &dataForUBO);

    BeginRender(HMM_V4(0, 0, 0, 0), thisExtractionCommandBuffer);
    Render(mesh, thisExtractionCommandBuffer);
    EndRender(thisExtractionCommandBuffer);

    if (vkEndCommandBuffer(thisExtractionCommandBuffer) != VK_SUCCESS) {
        throw - 1;
    }

    // Now we need to submit it
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // We only care about colour writing, this allows pre rasteriser to get head start
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &thisExtractionCommandBuffer;

    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

    if (vkQueueSubmit(Graphics::GetVkGraphicsQueue(), 1, &submitInfo, vkIsLastExtractionFinishedFence) != VK_SUCCESS) {
        throw - 1;
    }

    // Wait until above is finished. 
    vkWaitForFences(Graphics::GetVkDevice(), 1, &vkIsLastExtractionFinishedFence, VK_TRUE, UINT64_MAX);

    // Reset it 
    vkResetFences(Graphics::GetVkDevice(), 1, &vkIsLastExtractionFinishedFence);

    std::unique_ptr<std::vector<uint8_t>> data = positionImage.ExtractImageData();
    std::vector<HMM_Vec4> formattedPositionData(data->size() / sizeof(HMM_Vec4));
    for (int p = 0; p < data->size() / sizeof(HMM_Vec4); p++) {
        formattedPositionData[p] = *reinterpret_cast<HMM_Vec4*>(&(*data)[p * sizeof(HMM_Vec4)]);
    }
    data.release();
    data = colorImage.ExtractImageData();
    struct UNORMColor { uint8_t r, g, b, a; };
    std::vector<UNORMColor> formattedColorData(data->size() / sizeof(UNORMColor));
    for (int p = 0; p < data->size() / sizeof(UNORMColor); p++) {
        formattedColorData[p] = *reinterpret_cast<UNORMColor*>(&(*data)[p * sizeof(UNORMColor)]);
    }
    data.release();
    data = normalImage.ExtractImageData();
    std::vector<HMM_Vec4> formattedNormalData(data->size() / sizeof(HMM_Vec4));
    for (int p = 0; p < data->size() / sizeof(HMM_Vec4); p++) {
        formattedNormalData[p] = *reinterpret_cast<HMM_Vec4*>(&(*data)[p * sizeof(HMM_Vec4)]);
    }
    data.release();
    for (int p = 0; p < formattedPositionData.size(); p++) {
        if (formattedPositionData[p].A != 0) {
            output.points.push_back({ formattedPositionData[p].RGB , HMM_V3(formattedColorData[p].r / 255.0f, formattedColorData[p].g / 255.0f , formattedColorData[p].b / 255.0f ), formattedNormalData[p].RGB });
            output.indices.push_back(output.indices.size());
        }
    }
}

// THIS is too slow, dont use
void MeshRenderer::CollapsePoints()
{
    std::cout << "We had " << std::to_string(output.points.size()) << " points.\n";
    float twentieth = output.points.size() / 20.0f;
    float computed = 0;
    for (int i = 0; i < output.points.size(); i++) {
        uint32_t pointsRemoved = 0;
        for (int j = 0; j < output.points.size(); j++) {
            if (i == j) continue;
            if (HMM_LenSqrV3(output.points[i].position - output.points[j].position) < 0.00001f) {
                output.points.erase(output.points.begin() + j);
                j--;
            }
        }
        i -= pointsRemoved;
        computed++;
        if (computed > twentieth) {
            twentieth = output.points.size() / 20.0f;
            std::cout << "X";
            computed = 0;
        }
    }
    output.indices.resize(output.points.size());
    std::cout << "Now have " << std::to_string(output.points.size()) << " points.\n";
}

void MeshRenderer::TEMP_TestImageData()
{
    std::unique_ptr<std::vector<uint8_t>> data = positionImage.ExtractImageData();
    std::vector<HMM_Vec4> formattedData(data->size() / sizeof(HMM_Vec4));
    for (int p = 0; p < data->size() / sizeof(HMM_Vec4); p++) {
        formattedData[p] = *reinterpret_cast<HMM_Vec4*>(&(*data)[p * sizeof(HMM_Vec4)]);
    }
    data.release();

    for (int p = 0; p < formattedData.size(); p++) {
        if(formattedData[p].R != 0) std::cout << formattedData[p].R;
        if (p % 512 == 0) std::cout << "\n";
    }
}
