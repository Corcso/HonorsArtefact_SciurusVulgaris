#pragma once
#include "PCH.h"

/// <summary>
/// Points to Gbuffer Mesh Shader Graphics Pipeline
/// </summary>
class PointsToGBufferMeshShade_GP
{
public:
	/// <summary>
	/// Create the descriptor layout
	/// </summary>
	void CreateDescriptorLayout();
	/// <summary>
	/// Create the pipeline.
	/// </summary>
	/// <param name="vkRenderPass">Render pass pipeline will render on.</param>
	void CreatePipeline(const VkRenderPass& vkRenderPass);

	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkPipelineLayout;
	VkPipeline vkPipeline;

	void Shutdown();
};

