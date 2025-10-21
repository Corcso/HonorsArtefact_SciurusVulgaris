#pragma once
#include "PCH.h"

const std::vector<std::string> VK_DEVICE_EXTENSIONS_REQUIRED{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

class Graphics
{
public:
	Graphics() = default;
	Graphics(Graphics& copy) = delete;

	static void Initialize(int width, int height, std::wstring title);

	static VkInstance GetVkInstance() { return instance.vkInstance; }
	static VkSurfaceKHR GetVkSurface() { return instance.vkSurface; }
	static VkPhysicalDevice GetVkPhysicalDevice() { return instance.vkPhysicalDevice; }
	static VkDevice GetVkDevice() { return instance.vkDevice; }
	static VkQueue GetVkGraphicsQueue() { return instance.vkGraphicsQueue; }
	static VkQueue GetVkPresentQueue() { return instance.vkPresentQueue; }

private:
	static Graphics instance;

	const LPCWSTR WINDOW_CLASS_NAME = L"2200592-SciurusVulgaris";

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
	VkSwapchainKHR vkSwapChain;
	VkRenderPass vkRenderPass;
	VkDescriptorPool vkDescriptorPool;
};

