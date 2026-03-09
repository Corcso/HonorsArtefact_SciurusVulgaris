#pragma once
#include "PCH.h"
#include "PointTreeMesh.h"
#include "TriListMesh.h"
#include "Graphics.h"
#include "PointsToGBuffer_GP.h"
#include "GBufferToOutput_GP.h"
#include "MeshTraditionalToGBuffer_GP.h"
#include "PointsToGBufferMeshShade_GP.h"
#include "FXAA_GP.h"
#include "TAA_GP.h"

class InstancedTreeRenderPass
{
public:
	//void CreateDescriptorLayout();
	//void CreatePipeline();

	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();

	void CreateRenderPasses();

	void CreateTAAResources();

	void CreateAll() {
		//CreateDescriptorLayout();
		CreateImages();
		CreateSampler();
		CreateRenderPasses();
		CreateFrameBuffer();
		//CreatePipeline();
		CreateTAAResources();
		pointsToGBuffer_GP.CreateDescriptorLayout();
		pointsToGBuffer_GP.CreatePipeline(vkRenderPass);
		pointsToGBufferMeshShade_GP.CreateDescriptorLayout();
		pointsToGBufferMeshShade_GP.CreatePipeline(vkRenderPass);
		meshTraditionalToGBuffer_GP.CreateDescriptorLayout();
		meshTraditionalToGBuffer_GP.CreatePipeline(vkRenderPass);
		gBufferToOutput_GP.CreateDescriptorLayout();
		gBufferToOutput_GP.CreatePipeline(vkSecondRenderPass);
		fxaa_GP.CreateDescriptorLayout();
		fxaa_GP.CreatePipeline(Graphics::GetSwapChainRenderPass());
		taa_GP.CreateDescriptorLayout();
		taa_GP.CreatePipeline(TAARenderPass);

		
		
		CreateUniqueMeshData();
	}
	void Shutdown();

	void BeginRenderMeshShade(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void BeginRenderVertexShade(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderPointTree(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderPointTreeViaMeshShader(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void SwitchToTraditionalMeshPipeline(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderTraditionalMesh(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteFXAARender(bool enabled, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndFXAARender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExecuteTAARender(bool enabled, bool logarithmicColorSpace, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndTAARender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return pointsToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return pointsToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VkDescriptorSetLayout GetMeshShadeDescriptorSetLayout() { return pointsToGBufferMeshShade_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetMeshShadeDescriptorSetLayoutInfo() { return pointsToGBufferMeshShade_GP.vkDescriptorSetLayoutInfo; }

	VkDescriptorSetLayout GetMeshTraditionalDescriptorSetLayout() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetMeshTraditionalDescriptorSetLayoutInfo() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VulkanObjectDescriptorSet* GetQuadDescriptorSet() { return fullScreenQuad->GetDescriptorSet(); }

	void UpdateTAADescriptor(VulkanObjectDescriptorSet* descriptor, uint32_t binding, bool enabled, bool logarithmicColorSpace);

private:
	// First Pass
	VkRenderPass vkRenderPass;
	PointsToGBuffer_GP pointsToGBuffer_GP;
	GBufferToOutput_GP gBufferToOutput_GP;
	MeshTraditionalToGBuffer_GP meshTraditionalToGBuffer_GP;
	PointsToGBufferMeshShade_GP pointsToGBufferMeshShade_GP;
	//VkDescriptorSetLayout vkDescriptorSetLayout;
	//VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;
	Image velocityImage;
	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler; // Nearest Sampler (As Should be pixel = pixel) for performance.

	//VkPipelineLayout vkMainPipelineLayout;
	//VkPipeline vkMainPipeline;

	// Second Pass
	Image colorImageFinal;
	Image depthImageFinal;
	VkFramebuffer vkFrameBufferFinal;
	TriListMesh* fullScreenQuad;

	VkRenderPass vkSecondRenderPass;

	// == Third AA Pass ==

	// FXAA
	FXAA_GP fxaa_GP;
	VulkanObjectDescriptorSet fxaaDescriptor;
	FXAAInfo fxaaInfo;

	//TAA
	TAA_GP taa_GP;
	VulkanObjectDescriptorSet taaDescriptor;
	HMM_Vec2 TAAJitterValues[16];
	Image TAAHistoryImage;
	Image TAAOutputImage;
	VkFramebuffer TAAOutputImageFrameBuffer;
	VkRenderPass TAARenderPass;
};

