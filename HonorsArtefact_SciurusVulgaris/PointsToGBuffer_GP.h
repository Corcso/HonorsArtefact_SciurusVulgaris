#pragma once
#include "PCH.h"

class PointsToGBuffer_GP
{
public:
	void CreateDescriptorLayout();
	void CreatePipeline(const VkRenderPass& vkRenderPass);

	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkPipelineLayout;
	VkPipeline vkPipeline;

	void Shutdown();
};

