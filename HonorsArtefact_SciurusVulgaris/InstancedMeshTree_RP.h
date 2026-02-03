#pragma once
#include "PCH.h"
#include "Image.h"
#include "InstancedMeshTraditionalToGBuffer_GP.h"
#include "GBufferToOutput_GP.h"
#include "TriListMesh.h"
#include "Graphics.h"

class InstancedMeshTree_RP
{
public:
	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();
	void CreateRenderPass();

	void CreateAll() {
		CreateImages();
		CreateSampler();
		CreateRenderPass();
		CreateFrameBuffer();

		instancedMeshTraditionalToGBuffer_GP.CreateDescriptorLayout();
		instancedMeshTraditionalToGBuffer_GP.CreatePipeline(vkRenderPass);
		gBufferToOutput_GP.CreateDescriptorLayout();
		gBufferToOutput_GP.CreatePipeline(Graphics::GetSwapChainRenderPass());

		CreateUniqueMeshData();
	}

	void BeginRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderMeshTree(TriListMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return instancedMeshTraditionalToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return instancedMeshTraditionalToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VulkanObjectDescriptorSet* GetQuadDescriptorSet() { return fullScreenQuad->GetDescriptorSet(); }

	void Shutdown();
private:
	VkRenderPass vkRenderPass;
	GBufferToOutput_GP gBufferToOutput_GP;
	InstancedMeshTraditionalToGBuffer_GP instancedMeshTraditionalToGBuffer_GP;

	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;
	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler; // Nearest Sampler (As Should be pixel = pixel) for performance.

	// Second pass
	TriListMesh* fullScreenQuad;
};

