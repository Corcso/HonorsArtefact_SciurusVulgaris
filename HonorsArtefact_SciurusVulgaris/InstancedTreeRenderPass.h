#pragma once
#include "PCH.h"
#include "PointTreeMesh.h"
#include "TriListMesh.h"
#include "Graphics.h"
#include "PointsToGBuffer_GP.h"
#include "GBufferToOutput_GP.h"
#include "MeshTraditionalToGBuffer_GP.h"

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
		meshTraditionalToGBuffer_GP.CreateDescriptorLayout();
		meshTraditionalToGBuffer_GP.CreatePipeline(vkRenderPass);
		gBufferToOutput_GP.CreateDescriptorLayout();
		gBufferToOutput_GP.CreatePipeline(Graphics::GetSwapChainRenderPass());
		
		CreateUniqueMeshData();
	}
	void Shutdown();

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderPointTree(PointTreeMesh* points, uint32_t pointCountOverride, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void SwitchToTraditionalMeshPipeline(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderTraditionalMesh(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return pointsToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return pointsToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VkDescriptorSetLayout GetMeshTraditionalDescriptorSetLayout() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetMeshTraditionalDescriptorSetLayoutInfo() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VulkanObjectDescriptorSet* GetQuadDescriptorSet() { return fullScreenQuad->GetDescriptorSet(); }

private:
	// First Pass
	VkRenderPass vkRenderPass;
	PointsToGBuffer_GP pointsToGBuffer_GP;
	GBufferToOutput_GP gBufferToOutput_GP;
	MeshTraditionalToGBuffer_GP meshTraditionalToGBuffer_GP;
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

