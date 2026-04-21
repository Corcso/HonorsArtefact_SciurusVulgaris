#pragma once
#include "PCH.h"

/// <summary>
/// FXAA Graphics Pipeline
/// </summary>
class FXAA_GP
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
