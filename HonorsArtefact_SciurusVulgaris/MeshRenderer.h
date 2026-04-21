#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include "TriListMesh.h"
#include "PointTreeMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"
#include "BufferStructs.h"

class MeshRenderer
{
public:

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

	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void Render(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void ExtractPoints(TriListMesh** meshes, uint64_t meshCount, HMM_Vec3 viewingFrom, HMM_Vec3 upDirection);
	void ExtractPointsNew(std::vector<TriListMesh>* meshes, WCP_Matrices transformation);
	void CollapsePoints();

	VkImageView GetColorImageView() const { return colorImage.GetImageView(); }
	VkDescriptorSetLayout GetDescriptorSetLayout() const { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() const { return vkDescriptorSetLayoutInfo; }
	VkImageView GetPositionImageView() const { return positionImage.GetImageView(); }
	VkImageView GetNormalImageView() const { return normalImage.GetImageView(); }
	VkSampler GetSampler() const { return vkSampler; }
	PointTreeMesh* GetPointMeshOutput() { return output; }
private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

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

