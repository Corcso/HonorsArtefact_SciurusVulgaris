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
	VkSampler GetSampler() { return vkSampler; }
private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	Image colorImage;
	Image positionImage;
	Image depthImage;
	Image testImage;

	VkFramebuffer vkFrameBuffer;
	VkSampler vkSampler;


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

