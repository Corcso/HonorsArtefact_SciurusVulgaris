#pragma once
#include "PCH.h"
#include "VulkanDescriptor.h"
#include "PointMesh.h"
#include "MeshRenderer.h"
#include "Image.h"

/// <summary>
/// Required device extensions
/// </summary>
const std::vector<const char *> VK_DEVICE_EXTENSIONS_REQUIRED{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_MESH_SHADER_EXTENSION_NAME
};
constexpr int VULKAN_MAX_FRAMES_IN_FLIGHT = 2;

/// <summary>
/// Graphics Singleton
/// Manages the GPU global resources, Swap Chain and Window
/// </summary>
class Graphics
{
public:
	Graphics() = default;
	Graphics(Graphics& copy) = delete;

	/// <summary>
	/// Initialize and create window & GPU Resources
	/// </summary>
	/// <param name="width">Window width</param>
	/// <param name="height">Window height</param>
	/// <param name="title">Window Title</param>
	static void Initialize(int width, int height, std::wstring title);

	/// <summary>
	/// Same as vkWaitDeviceIdle(Graphics::GetVkDevice());
	/// </summary>
	static void WaitUntilGPUIdle();

	/// <summary>
	/// Delete all Singleton owned objects
	/// </summary>
	static void Shutdown();

	/// <summary>
	/// Begin the swap chain render
	/// </summary>
	static void BeginRender();
	/// <summary>
	/// Finish the ImGui Render causing ImGui to paint its windows
	/// </summary>
	static void FinishImGuiRender();
	/// <summary>
	/// End the swap chain render, and present the frame
	/// </summary>
	static void EndRender();

	static void RegisterWindowSizeChange(HMM_Vec2 newSize);
	static HMM_Vec2 GetWindowLocation();

	// Getters for all resources

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
	static VkRenderPass GetSwapChainRenderPass() { return instance.vkRenderPass; }
	static VkFramebuffer GetThisFramesFrameBuffer() { return instance.vkSwapChainFrameBuffers[instance.thisRenderImageIndex]; }
	static VkSampler GetBasicLinearSampler() { return instance.basicLinearSampler; }
	static VkSampler GetBasicNearestSampler() { return instance.basicNearestSampler; }
	static Image* GetNoShadowMapImage() { return &instance.noShadowMapImage; }
	static void SaveSwapChainImageToFile(std::string path);

	/// <summary>
	/// Check if a full performance capture run is active. 
	/// </summary>
	static bool IsFullCaptureRunActive() { return instance.fullCaptureModeEnabled; }

	static void CheckVulkanResult(VkResult res)
	{
		if (res == VK_SUCCESS)
			return;
		throw - 1;
	}

	static Graphics instance;
private:
	const LPCWSTR WINDOW_CLASS_NAME = L"2200592-SciurusVulgaris";

	// Win32 Window Handle
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
	/// <summary>
	/// Recreate the swapchain when the size changes
	/// </summary>
	static void RecreateSwapChain();
	bool swapChainNeedsRecreation; VkExtent2D newSwapChainSize;
	VkRenderPass vkRenderPass;

	// Memory allocator
	VulkanMemoryAllocator VRAMAllocator;

	// Descriptor pool
	VkDescriptorPool vkDescriptorPool;

	VkImage vkDepthImage;
	VkImageView vkDepthImageView;
	VkDeviceMemory vkDepthImageMemory;

	// Command pool & buffers per frame
	VkCommandPool vkCommandPool;
	std::vector<VkCommandBuffer> vkCommandBuffers;

	// Sync
	std::vector<VkFence> vkInFlightFences;
	std::vector<VkSemaphore> vkImageAvailableSemaphores;
	std::vector<VkSemaphore> vkRenderFinishedSemaphores;

	// Helpers
	VkSampler basicLinearSampler;
	VkSampler basicNearestSampler;
	Image noShadowMapImage;

	// Variables
	HMM_Vec4 clearColor{ 0.3f, 0.6f, 0.8f, 1.0f };

	int currentWidth, currentHeight;
	uint8_t currentFrame;
	uint32_t thisRenderImageIndex;
	uint64_t thisFramesDrawCall;

	bool fullCaptureModeEnabled = false; // Polled by other parts of the application to initiate a full capture then close the application. 
#ifdef NV_PERF_METER
	// NV PERF SPECIFIC
	nv::perf::profiler::ReportGeneratorVulkan nvperf_reportGenerator;

	nv::perf::sampler::PeriodicSamplerTimeHistoryVulkan nvperf_sampler;
	nv::perf::hud::HudPresets nvperf_hudPresets;
	nv::perf::hud::HudDataModel nvperf_hudDataModel;
	nv::perf::hud::HudImPlotRenderer nvperf_hudRenderer;

	nv::perf::ClockInfo nvperf_clockInfo;
	bool nvperf_InitiateReportNextFrame = false;

	bool nvperf_liveMode = false;

public:
	static void nvperf_InitiateReport(std::string folder);
	static std::string nfperf_GetLastReportDir();
#endif // NV_PERF_METER
public:
	/// <summary>
	/// Push a NV Perf metric range. 
	/// Does nothing if not on Metered build profile.
	/// </summary>
	/// <param name="name">Range name</param>
	static void PushMetricRange(std::string name);
	/// <summary>
	/// Pop a NV Perf metric range. 
	/// Does nothing if not on Metered build profile.
	/// </summary>
	static void PopMetricRange();

};

