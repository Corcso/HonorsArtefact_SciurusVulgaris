#pragma once
#include "VulkanMemoryAllocator.h"
#include "PointMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"


/// <summary>
/// UNUSED CLASS - DO NOT USE
/// </summary>
class PointRenderPipeline
{
public:
	void CreateAll() {
		//CreateImages();
		//CreateSampler();
		//CreateRenderPass();
		//CreateFrameBuffer();
		CreateDescriptorLayout();
		CreatePipeline();
		//CreateSyncObjects();
	}
	void Shutdown();

	//void CreateImages();
	//void CreateSampler();
	//void CreateRenderPass();
	//void CreateFrameBuffer();
	void CreateDescriptorLayout();
	void CreatePipeline();
	//void CreateSyncObjects();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(PointMesh* points, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
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

