#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"

class ImGuiBlankRenderPass
{
public:
	void CreateAll() {
	}
	void Shutdown();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }

private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;
};

