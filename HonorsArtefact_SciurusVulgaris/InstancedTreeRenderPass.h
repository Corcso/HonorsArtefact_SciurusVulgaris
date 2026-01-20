#pragma once
#include "PCH.h"
#include "PointTreeMesh.h"
#include "TriListMesh.h"
#include "Graphics.h"
#include "PointsToGBuffer_GP.h"
#include "GBufferToOutput_GP.h"

class InstancedTreeRenderPass
{
public:
	//void CreateDescriptorLayout();
	//void CreatePipeline();

	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();

	void CreateRenderPass();

	void CreateAll() {
		//CreateDescriptorLayout();
		CreateImages();
		CreateSampler();
		CreateRenderPass();
		CreateFrameBuffer();
		//CreatePipeline();
		pointsToGBuffer_GP.CreateDescriptorLayout();
		pointsToGBuffer_GP.CreatePipeline(vkRenderPass);
		gBufferToOutput_GP.CreateDescriptorLayout();
		gBufferToOutput_GP.CreatePipeline(Graphics::GetSwapChainRenderPass());
		
		CreateUniqueMeshData();
	}
	void Shutdown();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(PointTreeMesh* points, uint32_t pointCountOverride, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return pointsToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return pointsToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VulkanObjectDescriptorSet* GetQuadDescriptorSet() { return fullScreenQuad->GetDescriptorSet(); }

private:
	// First Pass
	VkRenderPass vkRenderPass;
	PointsToGBuffer_GP pointsToGBuffer_GP;
	GBufferToOutput_GP gBufferToOutput_GP;
	//VkDescriptorSetLayout vkDescriptorSetLayout;
	//VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;
	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler; // Nearest Sampler (As Should be pixel = pixel) for performance.

	//VkPipelineLayout vkMainPipelineLayout;
	//VkPipeline vkMainPipeline;

	// Second Pass
	TriListMesh* fullScreenQuad;

	VkRenderPass vkSecondRenderPass;
};

