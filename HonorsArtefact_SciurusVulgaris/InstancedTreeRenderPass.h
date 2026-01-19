#pragma once
#include "PCH.h"
#include "PointTreeMesh.h"
#include "TriListMesh.h"

class InstancedTreeRenderPass
{
public:
	void CreateDescriptorLayout();
	void CreatePipeline();

	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();

	void CreateRenderPass();
	void CreateSecondDescriptorLayout();
	void CreateSecondPipeline();

	void CreateAll() {
		CreateDescriptorLayout();
		CreateImages();
		CreateSampler();
		CreateRenderPass();
		CreateFrameBuffer();
		CreatePipeline();

		CreateSecondDescriptorLayout();
		CreateSecondPipeline();
		CreateUniqueMeshData();
	}
	void Shutdown();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(PointTreeMesh* points, uint32_t pointCountOverride, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }

private:
	// First Pass
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;
	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler; // Nearest Sampler (As Should be pixel = pixel) for performance.

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	// Second Pass
	TriListMesh* fullScreenQuad;

	VkRenderPass vkSecondRenderPass;
	VkDescriptorSetLayout vkSecondDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkSecondDescriptorSetLayoutInfo;

	VkPipelineLayout vkSecondPipelineLayout;
	VkPipeline vkSecondPipeline;
};

