#pragma once
#include "VulkanMemoryAllocator.h"
#include "TriListMesh.h"
#include "VulkanDescriptor.h"
#include "Image.h"

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

	VkImageView GetImageView() { return colorImage.GetImageView(); }
	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }
	VkImageView GetImageView2() { return positionImage.GetImageView(); }
	VkSampler GetSampler() { return vkSampler; }
private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	Image colorImage;
	Image positionImage;
	Image depthImage;

	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler;


	// TODO REMOVE AND TIDY
	int frameinc;

	struct WCP_Matrices {
		HMM_Mat4 world;
		HMM_Mat4 camera;
		HMM_Mat4 projection;
	};
	uint64_t thisFramesDrawCall;
};

