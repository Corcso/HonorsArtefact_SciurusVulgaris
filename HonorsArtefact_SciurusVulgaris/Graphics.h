#pragma once
#include "PCH.h"
#include "VulkanDescriptor.h"
#include "PointMesh.h"
#include "MeshRenderer.h"

const std::vector<std::string> VK_DEVICE_EXTENSIONS_REQUIRED{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
constexpr int VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

class Graphics
{
public:
	Graphics() = default;
	Graphics(Graphics& copy) = delete;

	static void Initialize(int width, int height, std::wstring title);
	static void WaitUntilGPUIdle();
	static void Shutdown();

	static void BeginRender();
	static void Render(PointMesh* points);
	static void EndRender();

	static VkInstance GetVkInstance() { return instance.vkInstance; }
	static VkSurfaceKHR GetVkSurface() { return instance.vkSurface; }
	static VkPhysicalDevice GetVkPhysicalDevice() { return instance.vkPhysicalDevice; }
	static VkDevice GetVkDevice() { return instance.vkDevice; }
	static VkQueue GetVkGraphicsQueue() { return instance.vkGraphicsQueue; }
	static VkQueue GetVkPresentQueue() { return instance.vkPresentQueue; }
	static VulkanMemoryAllocator& GetMemoryAllocator() { return instance.VRAMAllocator; }
	static VkCommandPool GetCommandPool() { return instance.vkCommandPool; }
	static VkFormat GetSwapChainFormat() { return instance.vkSwapChainFormat; }
	static VkExtent2D GetSwapChainExtent() { return instance.vkSwapChainExtent; }
	static VkCommandBuffer GetThisFramesCommandBuffer() { return instance.vkCommandBuffers[instance.currentFrame]; }
	static VkDescriptorPool GetDescriptorPool() { return instance.vkDescriptorPool; }
	static VkDescriptorSetLayout GetDescriptorSetLayout() { return instance.vkDescriptorSetLayout; }
	static VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return instance.vkDescriptorSetLayoutInfo; }

	//static void AddAdditionalDescriptorSet(std::vector<std::vector<VulkanObjectDescriptorSet>>& descriptorSetList, const VkDescriptorSetLayout& setLayout);

	static void CheckVulkanResult(VkResult res)
	{
		if (res == VK_SUCCESS)
			return;
		throw - 1;
	}
	MeshRenderer meshRenderer;
	static Graphics instance;
private:
	

	const LPCWSTR WINDOW_CLASS_NAME = L"2200592-SciurusVulgaris";

	int frameinc; // TODO REMOVE
	ImTextureID meshRenderOutput;
	ImTextureID meshRenderOutput2;
	ImTextureID meshRenderOutput3;
	
	TriListMesh* myMesh;
	Image myMeshImage;

	HWND window;

	VkInstance vkInstance;
	VkSurfaceKHR vkSurface;
	VkPhysicalDevice vkPhysicalDevice;
	VkDevice vkDevice;
	VkQueue vkGraphicsQueue;
	VkQueue vkPresentQueue;
	VkFormat vkSwapChainFormat;
	VkExtent2D vkSwapChainExtent;
	std::vector<VkImage> vkSwapChainImages;
	std::vector<VkImageView> vkSwapChainImageViews;
	std::vector<VkFramebuffer> vkSwapChainFrameBuffers;
	VkSwapchainKHR vkSwapChain;
	VkRenderPass vkRenderPass;

	// Memory allocator
	VulkanMemoryAllocator VRAMAllocator;

	// Descriptors
	VkDescriptorPool vkDescriptorPool;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;
	//std::vector<std::vector<VulkanDescriptor>> perFramePerObjectDescriptors;

	VkPipelineLayout vkMainPipelineLayout;
	VkPipeline vkMainPipeline;

	VkImage vkDepthImage;
	VkImageView vkDepthImageView;
	VkDeviceMemory vkDepthImageMemory;

	VkCommandPool vkCommandPool;
	std::vector<VkCommandBuffer> vkCommandBuffers;

	// Sync
	std::vector<VkFence> vkInFlightFences;
	std::vector<VkSemaphore> vkImageAvailableSemaphores;
	std::vector<VkSemaphore> vkRenderFinishedSemaphores;

	// Variables
	HMM_Vec4 clearColor{ 0, 0, 0, 0 };

	int currentWidth, currentHeight;
	uint8_t currentFrame;
	uint32_t thisRenderImageIndex;
	uint64_t thisFramesDrawCall;
};

