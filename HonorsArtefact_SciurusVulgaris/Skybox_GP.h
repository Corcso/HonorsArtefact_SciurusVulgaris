#pragma once
#include "PCH.h"

class Skybox_GP
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

