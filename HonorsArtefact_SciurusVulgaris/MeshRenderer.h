#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include "TriListMesh.h"
#include "PointTreeMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"
#include "BufferStructs.h"

/// <summary>
/// Triangle Mesh Renderer used in Generator App
/// <para>An old unified renderer OUR/UGP, Render passes and graphics pipelines combined into 1 class</para>
/// </summary>
class MeshRenderer
{
public:
	/// <summary>
	/// Create all objects
	/// </summary>
	void CreateAll() {
		CreateImages();
		CreateSampler();
		CreateRenderPass();
		CreateFrameBuffer();
		CreateDescriptorLayout();
		CreatePipeline();
		CreateSyncObjects();
		output = new PointTreeMesh();
	}
	void Shutdown();

	void CreateImages();
	void CreateSampler();
	void CreateRenderPass();
	void CreateFrameBuffer();
	void CreateDescriptorLayout();
	void CreatePipeline();
	void CreateSyncObjects();

	/// <summary>
	/// Begin the render pass
	/// </summary>
	/// <param name="clearColor">Clear colour</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Render a triangle mesh
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void Render(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// End the render pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// OLD Extract points function, do not use. 
	/// </summary>
	void ExtractPoints(TriListMesh** meshes, uint64_t meshCount, HMM_Vec3 viewingFrom, HMM_Vec3 upDirection);

	/// <summary>
	/// Extract points from the triangle mesh provided using virtual photogrammetry.
	/// <para>Call this for each angle you wish to capture</para>
	/// </summary>
	/// <param name="meshes">The vector of meshes to use. </param>
	/// <param name="transformation">WCP to apply.</param>
	void ExtractPointsNew(std::vector<TriListMesh>* meshes, WCP_Matrices transformation);

	/// <summary>
	/// A function which collapses close points in the point mesh output. DO NOT USE. It is very slow and not needed as LODs are selected based on random order. 
	/// </summary>
	void CollapsePoints();

	VkImageView GetColorImageView() const { return colorImage.GetImageView(); }
	VkDescriptorSetLayout GetDescriptorSetLayout() const { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() const { return vkDescriptorSetLayoutInfo; }
	VkImageView GetPositionImageView() const { return positionImage.GetImageView(); }
	VkImageView GetNormalImageView() const { return normalImage.GetImageView(); }
	VkSampler GetSampler() const { return vkSampler; }

	/// <summary>
	/// Returns the output point mesh generated from extracting points. 
	/// </summary>
	PointTreeMesh* GetPointMeshOutput() { return output; }
private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	// Images used for extraction
	Image colorImage;
	Image positionImage;
	Image normalImage;
	Image depthImage;

	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler;

	VkFence vkIsLastExtractionFinishedFence;
	PointTreeMesh* output;
	
	uint64_t thisFramesDrawCall;
};

