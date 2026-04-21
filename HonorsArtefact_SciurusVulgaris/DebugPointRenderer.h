#pragma once
#include "VulkanMemoryAllocator.h"
#include "PointTreeMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"

/// <summary>
/// Debug point renderer, used in the LOD view page in the generation app. 
/// <para>An old unified renderer OUR/UGP, Render passes and graphics pipelines combined into 1 class</para>
/// </summary>
class DebugPointRenderer
{
public:
	/// <summary>
	/// Create all objects
	/// </summary>
	void CreateAll() {
		CreateImages();
		CreateRenderPass();
		CreateFrameBuffer();
		CreateDescriptorLayout();
		CreatePipeline();
	}
	void Shutdown();

	void CreateImages();
	void CreateRenderPass();
	void CreateFrameBuffer();
	void CreateDescriptorLayout();
	void CreatePipeline();

	/// <summary>
	/// Begin render.
	/// </summary>
	/// <param name="clearColor">Clear Colour</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// Render a point tree. 
	/// </summary>
	/// <param name="points">Point mesh to render</param>
	/// <param name="view">Rect View</param>
	/// <param name="LODLevel">LOD Level</param>
	/// <param name="descriptorSet">Optional, override for descriptor set</param>
	/// <param name="distanceToUse">Distance to use in LOD calculations</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void Render(PointTreeMesh* points, VkRect2D view, unsigned int LODLevel = 0, VulkanObjectDescriptorSet* descriptorSet = nullptr, float distanceToUse = 0 , VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	/// <summary>
	/// End the render
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
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

