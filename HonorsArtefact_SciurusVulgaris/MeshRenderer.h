#pragma once
#include "VulkanMemoryAllocator.h"
#include "TriListMesh.h"
#include "VulkanDescriptor.h"

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
	}
	void Shutdown();

	void CreateImages();
	void CreateSampler();
	void CreateRenderPass();
	void CreateFrameBuffer();
	void CreateDescriptorLayout();
	void CreatePipeline();

	void BeginRender(HMM_Vec4 clearColor);
	void Render(TriListMesh* mesh);
	void EndRender();

	VkImageView GetImageView() { return vkColorImageView; }
	VkSampler GetSampler() { return vkSampler; }
private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	VkImage vkColorImage;
	VkImageView vkColorImageView;
	VkFormat vkColorImageFormat;
	VkExtent2D vkColorImageExtent;
	VkFramebuffer vkFrameBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock vkColorImageMemory;

	VkImage vkPositionImage;
	VkImageView vkPositionImageView;
	VkFormat vkPositionImageFormat;
	VkExtent2D vkPositionImageExtent;
	VulkanMemoryAllocator::VulkanMemoryBlock vkPositionImageMemory;

	VkImage vkDepthImage;
	VkImageView vkDepthImageView;
	VkSampler vkSampler;
	VulkanMemoryAllocator::VulkanMemoryBlock vkDepthImageMemory;

	// TODO REMOVE AND TIDY
	int frameinc;

	struct WCP_Matrices {
		HMM_Mat4 world;
		HMM_Mat4 camera;
		HMM_Mat4 projection;
	};
	std::vector<VulkanDescriptor> perObjectDescriptors;
	uint64_t thisFramesDrawCall;
};

