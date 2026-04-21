#pragma once
#include "PCH.h"
#include "Image.h"
#include "InstancedMeshTraditionalToGBuffer_GP.h"
#include "GBufferToOutput_GP.h"
#include "TriListMesh.h"
#include "Graphics.h"

/// <summary>
/// Render pass for triangle trees.
/// </summary>
class InstancedMeshTree_RP
{
public:
	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();
	void CreateRenderPass();

	/// <summary>
	/// Create all resources and initialise all pipelines
	/// </summary>
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
	/// <summary>
	/// Begin the render pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Render a triangle tree
	/// </summary>
	/// <param name="mesh">Mesh to render</param>
	/// <param name="instancingInfo">Instancing information struct</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void RenderMeshTree(TriListMesh* mesh, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// End the render pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Execute Geometry paint pass. Colouring & Lighting the GBuffer data. 
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// End the Geometry paint pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
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

