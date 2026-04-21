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
#include "Skybox_GP.h"
#include "Transform.h"

/// <summary>
/// Instanced point tree rendering. Also renders terrain data. Supports vertex and mesh shading for points. 
/// </summary>
class InstancedTreeRenderPass
{
public:

	void CreateImages();
	void CreateUniqueMeshData();
	void CreateSampler();
	void CreateFrameBuffer();

	void CreateRenderPasses();

	void CreateTAAResources();

	/// <summary>
	/// Create all resources and setup all piplines
	/// </summary>
	void CreateAll() {
		CreateImages();
		CreateSampler();
		CreateRenderPasses();
		CreateFrameBuffer();
		CreateTAAResources();
		pointsToGBuffer_GP.CreateDescriptorLayout();
		pointsToGBuffer_GP.CreatePipeline(vkRenderPass);
		pointsToGBufferMeshShade_GP.CreateDescriptorLayout();
		pointsToGBufferMeshShade_GP.CreatePipeline(vkRenderPass);
		meshTraditionalToGBuffer_GP.CreateDescriptorLayout();
		meshTraditionalToGBuffer_GP.CreatePipeline(vkRenderPass);
		skybox_GP.CreateDescriptorLayout();
		skybox_GP.CreatePipeline(vkSecondRenderPass);
		CreateSkyboxMeshAndImage();
		gBufferToOutput_GP.CreateDescriptorLayout();
		gBufferToOutput_GP.CreatePipeline(vkSecondRenderPass);
		fxaa_GP.CreateDescriptorLayout();
		fxaa_GP.CreatePipeline(Graphics::GetSwapChainRenderPass());
		taa_GP.CreateDescriptorLayout();
		taa_GP.CreatePipeline(TAARenderPass);

		
		
		CreateUniqueMeshData();
	}
	void Shutdown();

	/// <summary>
	/// Update camera transform information for skybox rendering. 
	/// </summary>
	void UpdateCameraInfoForSkybox(CameraTransform cameraTransform);

	/// <summary>
	/// Begin point rendering with the mesh shader. 
	/// </summary>
	/// <param name="clearColor">Clear Color</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRenderMeshShade(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Begin point rendering with the vertex shader. 
	/// </summary>
	/// <param name="clearColor">Clear Color</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRenderVertexShade(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// Render a point tree using the vertex shader, make sure to call the relevant begin function first. 
	/// </summary>
	/// <param name="points">Point Mesh </param>
	/// <param name="instancingInfo">Instancing information</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void RenderPointTree(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// Render a point tree using the mesh shader, make sure to call the relevant begin function first. 
	/// </summary>
	/// <param name="points">Point Mesh </param>
	/// <param name="instancingInfo">Instancing information</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void RenderPointTreeViaMeshShader(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// Switch to the triangle mesh renderer pipeline for the terrain. Uses within the same render pass as the points. 
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void SwitchToTraditionalMeshPipeline(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	
	/// <summary>
	/// Render triangle mesh, call after SwitchToTraditionalMeshPipeline
	/// </summary>
	///<param name="mesh">Mesh to render </param>
	///<param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void RenderTraditionalMesh(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// End the first render pass of point and triangle data. 
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Execute Geometry paint pass. Colouring & Lighting the GBuffer data. 
	/// <para>Also renders the skybox if enabled</para>
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void ExecuteSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// End the Geometry paint pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndSecondRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Execute the FXAA Render, should come last. 
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void ExecuteFXAARender(bool enabled, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// End the FXAA Render, call ImGui render before this as this is the last render pass. 
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndFXAARender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Execute TAA render pass. Comes before TAA and after GBuffer paint.
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void ExecuteTAARender(bool enabled, bool logarithmicColorSpace, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// End the TAA Render
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndTAARender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return pointsToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return pointsToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VkDescriptorSetLayout GetMeshShadeDescriptorSetLayout() { return pointsToGBufferMeshShade_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetMeshShadeDescriptorSetLayoutInfo() { return pointsToGBufferMeshShade_GP.vkDescriptorSetLayoutInfo; }

	VkDescriptorSetLayout GetMeshTraditionalDescriptorSetLayout() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetMeshTraditionalDescriptorSetLayoutInfo() { return meshTraditionalToGBuffer_GP.vkDescriptorSetLayoutInfo; }

	VulkanObjectDescriptorSet* GetQuadDescriptorSet() { return fullScreenQuad->GetDescriptorSet(); }

	void UpdateTAADescriptor(VulkanObjectDescriptorSet* descriptor, uint32_t binding, bool enabled, bool logarithmicColorSpace);

	bool enableSkybox;
private:
	// First Pass
	VkRenderPass vkRenderPass;
	PointsToGBuffer_GP pointsToGBuffer_GP;
	GBufferToOutput_GP gBufferToOutput_GP;
	MeshTraditionalToGBuffer_GP meshTraditionalToGBuffer_GP;
	PointsToGBufferMeshShade_GP pointsToGBufferMeshShade_GP;
	Skybox_GP skybox_GP;

	// GBuffers
	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;
	Image velocityImage;
	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler; // Nearest Sampler (As Should be pixel = pixel) for performance.

	// Second Pass (Geometry Paint)
	Image colorImageFinal;
	Image depthImageFinal;
	VkFramebuffer vkFrameBufferFinal;
	TriListMesh* fullScreenQuad;

	VkRenderPass vkSecondRenderPass;

	// == AA Passes ==

	// FXAA (Forth) (output is swap chain)
	FXAA_GP fxaa_GP;
	VulkanObjectDescriptorSet fxaaDescriptor;
	FXAAInfo fxaaInfo;

	//TAA (Third)
	TAA_GP taa_GP;
	VulkanObjectDescriptorSet taaDescriptor;
	HMM_Vec2 TAAJitterValues[16];
	Image TAAHistoryImage;
	Image TAAOutputImage;
	VkFramebuffer TAAOutputImageFrameBuffer;
	VkRenderPass TAARenderPass;

	// Skybox
	Image activeSkyboxImage;
	TriListMesh* skyboxMesh;
	void CreateSkyboxMeshAndImage();
};

