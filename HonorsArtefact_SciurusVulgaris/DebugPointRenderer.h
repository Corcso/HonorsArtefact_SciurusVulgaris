#pragma once
#include "VulkanMemoryAllocator.h"
#include "PointMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"

class DebugPointRenderer
{
public:
	void CreateAll() {
		CreateImages();
		CreateRenderPass();
		CreateFrameBuffer();
		CreateDescriptorLayout();
		CreatePipeline();
		//CreateSyncObjects();
	}
	void Shutdown();

	void CreateImages();
	void CreateRenderPass();
	void CreateFrameBuffer();
	void CreateDescriptorLayout();
	void CreatePipeline();
	//void CreateSyncObjects();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(PointMesh* points, VkRect2D view, VulkanObjectDescriptorSet* descriptorSet = nullptr, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }
	ImTextureID GetImGuiOutputTexture() { return outputImageImGuiTex; }

private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	// Output Image
	Image outputImage;
	Image depthImage;
	ImTextureID outputImageImGuiTex;
	VkFramebuffer outputFrameBuffer;
};

