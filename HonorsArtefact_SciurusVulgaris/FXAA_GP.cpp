#include "PCH.h"
#include "FXAA_GP.h"
#include "Graphics.h"
#include "VulkanUtility.h"

void FXAA_GP::CreateDescriptorLayout()
{
    VkDescriptorSetLayoutBinding* uboLayoutBindings = new VkDescriptorSetLayoutBinding[2]; // Freed upon shutdown
    uboLayoutBindings[0].binding = 0;
    uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    uboLayoutBindings[0].descriptorCount = 1;
    uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBindings[0].pImmutableSamplers = nullptr;

    uboLayoutBindings[1].binding = 1;
    uboLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBindings[1].descriptorCount = 1;
    uboLayoutBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uboLayoutBindings[1].pImmutableSamplers = nullptr;

    vkDescriptorSetLayoutInfo = {};
    vkDescriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutInfo.bindingCount = 2;
    vkDescriptorSetLayoutInfo.pBindings = uboLayoutBindings;

    if (vkCreateDescriptorSetLayout(Graphics::GetVkDevice(), &vkDescriptorSetLayoutInfo, nullptr, &vkDescriptorSetLayout) != VK_SUCCESS) {
        throw - 1;
    }
}

void FXAA_GP::CreatePipeline(const VkRenderPass& vkRenderPass)
{
    // Get shader code

    auto vertShaderCode = VulkanUtility::ReadFile("./COMPILEDSHADER_DeferedQuad.spv");
    auto fragShaderCode = VulkanUtility::ReadFile("./COMPILEDSHADER_FXAAFrag.spv");

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
    // Hardcoded vertices for now so no CPU to GPU pass
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkVertexInputBindingDescription bindingDesc = vertexBindingDescription;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDesc; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptionCount;
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions; // Optional

    // Setup input assembler
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Setup viewport see section in https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)Graphics::GetSwapChainExtent().width;
    viewport.height = (float)Graphics::GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    // Scissor rectangle is the area which isnt discarded by the rasterizer, we want whole viewport for now
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = Graphics::GetSwapChainExtent();

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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
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

    // Skipping depth & stencil 

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

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
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

    if (vkCreatePipelineLayout(Graphics::GetVkDevice(), &pipelineLayoutInfo, nullptr, &vkPipelineLayout) != VK_SUCCESS) {
        throw - 1;
    }
    VulkanUtility::DebugNameObject("FXAA_GP_PipelineLayout", reinterpret_cast<uint64_t>(vkPipelineLayout), VK_OBJECT_TYPE_PIPELINE_LAYOUT);

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

    pipelineInfo.layout = vkPipelineLayout;

    pipelineInfo.renderPass = vkRenderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(Graphics::GetVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vkPipeline) != VK_SUCCESS) {
        throw - 1;
    }
    VulkanUtility::DebugNameObject("FXAA_GP_Pipeline", reinterpret_cast<uint64_t>(vkPipeline), VK_OBJECT_TYPE_PIPELINE);

    vkDestroyShaderModule(Graphics::GetVkDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(Graphics::GetVkDevice(), vertShaderModule, nullptr);
}

void FXAA_GP::Shutdown() {
    vkDestroyPipeline(Graphics::GetVkDevice(), vkPipeline, nullptr);
    vkDestroyPipelineLayout(Graphics::GetVkDevice(), vkPipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(Graphics::GetVkDevice(), vkDescriptorSetLayout, nullptr);
    delete[] vkDescriptorSetLayoutInfo.pBindings;
}