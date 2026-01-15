#pragma once
#include "PCH.h"
#include "PointTreeMesh.h"

class InstancedTreeRenderPass
{
public:
	void CreateDescriptorLayout();
	void CreatePipeline();

	void CreateAll() {
		CreateDescriptorLayout();
		CreatePipeline();
	}
	void Shutdown();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(PointTreeMesh* points, uint32_t pointCountOverride, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }

private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;
};

