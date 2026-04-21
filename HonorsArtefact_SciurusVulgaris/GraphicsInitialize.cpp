#include "PCH.h"
#include "Graphics.h"
#include "VulkanUtility.h"
#include "VulkanSetup.h"
#include "Input.h"
#include "ImGuiHelpers.h"

// The setup process is so long it gets its own CPP file.

void Graphics::Initialize(int width, int height, std::wstring title)
{
    // Begin Windows Window Setup

    // Define and register window class with OS
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = &Input::WndProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.lpszMenuName = nullptr;
    windowClass.lpszClassName = instance.WINDOW_CLASS_NAME;

    if (!RegisterClassEx(&windowClass)) {
        throw 2;
    }

    // Setup window

    // Client rect is the size of the renderable area (entire window)
    RECT clientRect = { 0, 0, width, height };
    // Window rect is the client rect asjusted for the top bar
    RECT windowRect = clientRect;
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    instance.currentHeight = clientRect.bottom;
    instance.currentWidth = clientRect.right;

    instance.window = CreateWindowW(instance.WINDOW_CLASS_NAME, title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        nullptr, nullptr, GetModuleHandle(NULL), nullptr);

    if (!instance.window) {
        throw 3;
    }

    ShowWindow(instance.window, SW_NORMAL);
    UpdateWindow(instance.window);

    //---------------------
    // Setup raw input for HD mouse movement (Microsoft, 2023)
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement
    #ifndef HID_USAGE_PAGE_GENERIC
    #define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
    #endif
    #ifndef HID_USAGE_GENERIC_MOUSE
    #define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
    #endif

    RAWINPUTDEVICE Rid[1];
    Rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
    Rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
    Rid[0].dwFlags = RIDEV_INPUTSINK;
    Rid[0].hwndTarget = instance.window;
    RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));
    //---------------------


    // Im gui init
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(instance.window);


    // Begin Vulkan Setup

    // >>> Create instance
    // First of all, check if we are having validation layers and set them up
    bool enableValidationLayers = false;
#ifdef _DEBUG
    enableValidationLayers = true;

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Validation layers we are looking for 
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            enableValidationLayers = false;
        }
    }
#endif
#ifdef NV_PERF_METER
    // Disable them if we are on a Metered build
    enableValidationLayers = false;
#endif // NV_PERF_METER


    // Setup app info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "2200592-SciurusVulgaris";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2; // 1.2 requried for mesh shaders

    // Setup instance creation info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // We are having 2 extensions, windows and surface
    std::vector<const char*> extensionsNames = {
        "VK_KHR_surface", "VK_KHR_win32_surface", "VK_EXT_debug_utils"
    };

#ifdef NV_PERF_METER
    // Append NV Perf requried extensions to instance list. 
    nv::perf::VulkanAppendInstanceRequiredExtensions(extensionsNames, appInfo.apiVersion);
#endif // NV_PERF_METER

    createInfo.enabledExtensionCount = extensionsNames.size();
    createInfo.ppEnabledExtensionNames = extensionsNames.data();

    // Validation layers layeers
    if (enableValidationLayers) {
#ifdef _DEBUG
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
#endif // _DEBUG
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    // Create the instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance.vkInstance);
    if (result != VK_SUCCESS) {
        throw - 1;
    }

    // >>> Create surface
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR; // On windows
    surfaceCreateInfo.hwnd = instance.window; // Our window
    surfaceCreateInfo.hinstance = GetModuleHandle(nullptr); // Hinstance

    if (vkCreateWin32SurfaceKHR(instance.vkInstance, &surfaceCreateInfo, nullptr, &instance.vkSurface) != VK_SUCCESS) {
        throw - 1;
    }

    // >>> Pick Device for rendering
    // Get and check device count
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance.vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }
    // Get all device handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance.vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);
        bool isSuitable = VulkanSetup::CheckDeviceSuitability(device, instance.vkSurface);
        std::cout << "Device : " << props.deviceName << " Suitable: " << ((isSuitable) ? "YES" : "NO") << "\n";
    }
    if (deviceCount > 1) {
        std::cout << "Enter Chosen Index: ";
        std::string choice;
        std::cin >> choice;
        if (std::stoi(choice) >= deviceCount) return; // Fail!

        instance.vkPhysicalDevice = devices[std::stoi(choice)];
    }
    // If you only have 1 device, it will choose that one.
    else {
        std::cout << "Choosing Only Device Automatically...\n";
        if(bool isSuitable = VulkanSetup::CheckDeviceSuitability(devices[0], instance.vkSurface)) instance.vkPhysicalDevice = devices[0];
    }
    if (instance.vkPhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    // >>> Create logical device & queues
    // Device queue
    VulkanSetup::QueueFamilyIndices indices = VulkanSetup::GetQueueFamilyIndices(instance.vkPhysicalDevice, instance.vkSurface);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<uint32_t> uniqueQueueFamilies;
    if (indices.graphicsFamily == indices.presentFamily) uniqueQueueFamilies = { indices.graphicsFamily };
    else uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Enable mesh shader features for device
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures{};
    meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    meshShaderFeatures.meshShader = true;
    meshShaderFeatures.taskShader = true;
    meshShaderFeatures.multiviewMeshShader = false;
    meshShaderFeatures.primitiveFragmentShadingRateMeshShader = false;
    meshShaderFeatures.meshShaderQueries = false;
    meshShaderFeatures.pNext = nullptr;

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkPhysicalDeviceFeatures2 deviceFeatures2{};
    deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    deviceFeatures2.features = deviceFeatures;
    deviceFeatures2.pNext = &meshShaderFeatures;
    

    // Device info
    VkDeviceCreateInfo logicDeviceCreateInfo{};
    logicDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    logicDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    logicDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    logicDeviceCreateInfo.pEnabledFeatures = nullptr;
    logicDeviceCreateInfo.pNext = &deviceFeatures2;

    std::vector<const char*> deviceExtensionsNames = VK_DEVICE_EXTENSIONS_REQUIRED;

#ifdef NV_PERF_METER
    // Append NV Perf requried extensions to device list. 
    nv::perf::VulkanAppendDeviceRequiredExtensions(instance.vkInstance, instance.vkPhysicalDevice, vkGetInstanceProcAddr(instance.vkInstance, "vkGetInstanceProcAddr"), deviceExtensionsNames);
    nv::perf::sampler::PeriodicSamplerTimeHistoryVulkan::AppendDeviceRequiredExtensions(appInfo.apiVersion, deviceExtensionsNames);
#endif // NV_PERF_METER

    logicDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsNames.size());
    logicDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsNames.data();

    if (vkCreateDevice(instance.vkPhysicalDevice, &logicDeviceCreateInfo, nullptr, &instance.vkDevice) != VK_SUCCESS) {
        throw - 1;
    }
    // Get Queues
    vkGetDeviceQueue(instance.vkDevice, indices.graphicsFamily, 0, &instance.vkGraphicsQueue);
    vkGetDeviceQueue(instance.vkDevice, indices.presentFamily, 0, &instance.vkPresentQueue);

    // >>> Create swap chain
    VulkanSetup::CreateSwapChain(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSurface, instance.currentWidth, instance.currentHeight, &instance.vkSwapChainFormat, &instance.vkSwapChainExtent, &instance.vkSwapChainImages, &instance.vkSwapChain);
    VulkanSetup::CreateImageViewsForSwapChain(instance.vkDevice, instance.vkSwapChainFormat, instance.vkSwapChainImages, &instance.vkSwapChainImageViews);

    // Setup render pass
    VulkanSetup::CreateRenderPass(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSwapChainFormat, &instance.vkRenderPass);

    // Setup descriptor pool
    VulkanSetup::CreateDescriptorPool(instance.vkDevice, 100, VULKAN_MAX_FRAMES_IN_FLIGHT * 100, &instance.vkDescriptorPool);

    // Setup depth buffer
    VulkanSetup::CreateDepthBuffer(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSwapChainExtent, &instance.vkDepthImage,
        &instance.vkDepthImageMemory, &instance.vkDepthImageView);

    // Setup frame buffers
    VulkanSetup::CreateFrameBuffers(instance.vkDevice, instance.vkRenderPass, instance.vkSwapChainExtent, instance.vkSwapChainImageViews, instance.vkDepthImageView, &instance.vkSwapChainFrameBuffers);

    // Setup command pool & buffers
    VulkanSetup::CreateCommandPool(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSurface, &instance.vkCommandPool);
    VulkanSetup::CreateCommandBuffers(instance.vkDevice, instance.vkCommandPool, &instance.vkCommandBuffers);

    // Create Sync objects
    VulkanSetup::CreateSyncObjects(instance.vkDevice, instance.vkSwapChainImages.size(), &instance.vkInFlightFences, &instance.vkImageAvailableSemaphores, &instance.vkRenderFinishedSemaphores);

    // Editor only ImGui Setup
    ImGui_ImplVulkan_InitInfo init_info = {};
    //init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
    init_info.Instance = instance.vkInstance;
    init_info.PhysicalDevice = instance.vkPhysicalDevice;
    init_info.Device = instance.vkDevice;
    init_info.QueueFamily = indices.graphicsFamily;
    init_info.Queue = instance.vkGraphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = instance.vkDescriptorPool;
    init_info.RenderPass = instance.vkRenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = instance.vkSwapChainImages.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = CheckVulkanResult;
    ImGui_ImplVulkan_Init(&init_info);

    // Create basic samplers
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(Graphics::GetVkDevice(), &samplerInfo, nullptr, &instance.basicLinearSampler) != VK_SUCCESS) {
        throw - 1;
    }

    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;

    if (vkCreateSampler(Graphics::GetVkDevice(), &samplerInfo, nullptr, &instance.basicNearestSampler) != VK_SUCCESS) {
        throw - 1;
    }

    // Create a 1x1 shadow map so buffers don't complain
    instance.noShadowMapImage.CreateImage(VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()), 1, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    instance.noShadowMapImage.CreateImageView(true);
    instance.noShadowMapImage.TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

#ifdef NV_PERF_METER
    // Ask if user wants a full runthrough capture, this means the program will autoload capture data, run a capture and close. 
    std::cout << "Would you like a full runthrough capture? (Y/N) : ";
    std::string runthroughCaptureAnswer;
    std::cin >> runthroughCaptureAnswer;
    if (runthroughCaptureAnswer == "Y" || runthroughCaptureAnswer == "y") {
        instance.fullCaptureModeEnabled = true;
        std::cout << "The program will now preform a full capture run. Please make sure Summer Bubble is your generated point mesh with at least 512 points. (This is the default when first downloading the application.\n";
        std::cout << "Also please close combined.csv if you already have it open!\nSit back, relax, and enjoy the capture : )\n";
        std::cout << "\nRenders will be saved to imageout/XXXXX.bmp, metrics to nvperfout/combined.csv\n\n";
    }
    // Setup NV Perf
    nv::perf::InitializeNvPerf();
    instance.nvperf_reportGenerator.additionalMetrics = { "crop__write_throughput" };
    instance.nvperf_reportGenerator.InitializeReportGenerator(instance.vkInstance, instance.vkPhysicalDevice, instance.vkDevice);
    instance.nvperf_reportGenerator.SetFrameLevelRangeName("Frame");
    instance.nvperf_reportGenerator.SetNumNestingLevels(10);
    instance.nvperf_reportGenerator.outputOptions.directoryName = "nvperfout\\metricpass";
    instance.nvperf_reportGenerator.outputOptions.enableCsvReport = true;
    instance.nvperf_reportGenerator.outputOptions.enableHtmlReport = true;
    instance.nvperf_reportGenerator.outputOptions.appendDateTimeToDirName = nv::perf::AppendDateTime::no;

    instance.nvperf_clockInfo = nv::perf::VulkanGetDeviceClockState(instance.vkInstance, instance.vkPhysicalDevice, instance.vkDevice);
    nv::perf::VulkanSetDeviceClockState(instance.vkInstance, instance.vkPhysicalDevice, instance.vkDevice, NVPW_DEVICE_CLOCK_SETTING_LOCK_TO_RATED_TDP);

    // Ask the user if they want live statistics, these cannot be enabled if the report generator is to be used. 
    std::cout << "Would you like live statistics? (Y/N) : ";
    std::string liveStatisticsEnabled;
    if(instance.fullCaptureModeEnabled){
        std::cout << "N\n";
        liveStatisticsEnabled == "N";
    }
    else {
        std::cin >> liveStatisticsEnabled;
    }
    // LIVE STATS SETUP
    if (liveStatisticsEnabled == "Y" || liveStatisticsEnabled == "y") {
        instance.nvperf_liveMode = true;

        instance.nvperf_sampler.Initialize(instance.vkInstance, instance.vkPhysicalDevice, instance.vkDevice);
        const nv::perf::DeviceIdentifiers deviceIdentifiers = instance.nvperf_sampler.GetGpuDeviceIdentifiers();
        const uint32_t                    maxFrameLatency = instance.vkSwapChainImages.size() + 1;
        instance.nvperf_sampler.BeginSession(instance.vkGraphicsQueue, indices.graphicsFamily, 1000 * 1000 * 1000 / 60, 1000U * 1000U * 1000U * 60U, maxFrameLatency);

        instance.nvperf_hudPresets.Initialize(deviceIdentifiers.pChipName);
        for (auto& preset : instance.nvperf_hudPresets.GetPresets()) {
            std::cout << preset.name << "\n";

        }
        instance.nvperf_hudDataModel.Load(instance.nvperf_hudPresets.GetPreset("Graphics General Triage"));

        std::string                  metricConfigName;
        nv::perf::MetricConfigObject metricConfigObject;
        if (nv::perf::MetricConfigurations::GetMetricConfigNameBasedOnHudConfigurationName(metricConfigName, deviceIdentifiers.pChipName, "Graphics General Triage"))
        {
            nv::perf::MetricConfigurations::LoadMetricConfigObject(metricConfigObject, deviceIdentifiers.pChipName, metricConfigName);
        }
        instance.nvperf_hudDataModel.Initialize(1.0 / (double)60, 4, metricConfigObject);
        instance.nvperf_sampler.SetConfig(&instance.nvperf_hudDataModel.GetCounterConfiguration());
        instance.nvperf_hudDataModel.PrepareSampleProcessing(instance.nvperf_sampler.GetCounterData());

        // initialize renderer
        ImPlot::CreateContext();
        nv::perf::hud::HudImPlotRenderer::SetStyle();

        instance.nvperf_hudRenderer.Initialize(instance.nvperf_hudDataModel);
    }
#endif // NV_PERF_METER

    // Set ImGui Style
    ImGuiHelpers::ImGuiSetupStyle();

    return ;
}

void Graphics::WaitUntilGPUIdle()
{
    // Wait until all processes complete, this doesn't work for present, need an extension for that
    vkDeviceWaitIdle(instance.vkDevice);
}

void Graphics::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Destroy Sync Objects
    for (auto& thisSemaphore : instance.vkRenderFinishedSemaphores) vkDestroySemaphore(instance.vkDevice, thisSemaphore, nullptr);
    for (auto& thisSemaphore : instance.vkImageAvailableSemaphores) vkDestroySemaphore(instance.vkDevice, thisSemaphore, nullptr);
    for (auto& thisFence : instance.vkInFlightFences) vkDestroyFence(instance.vkDevice, thisFence, nullptr);

    // Destroy Command pool & buffers
    vkDestroyCommandPool(instance.vkDevice, instance.vkCommandPool, nullptr);

    // Destroy Helpers
    instance.noShadowMapImage.Destroy();
    vkDestroySampler(instance.vkDevice, instance.basicLinearSampler, nullptr);
    vkDestroySampler(instance.vkDevice, instance.basicNearestSampler, nullptr);

    // Destroy Frame Buffers
    for(auto& thisFrameBuffer : instance.vkSwapChainFrameBuffers) vkDestroyFramebuffer(instance.vkDevice, thisFrameBuffer, nullptr);

    // Destroy Render Pass
    vkDestroyRenderPass(instance.vkDevice, instance.vkRenderPass, nullptr);

    // Destroy Images & Swap Chain
    for (auto& thisImageView : instance.vkSwapChainImageViews) vkDestroyImageView(instance.vkDevice, thisImageView, nullptr);
    vkDestroySwapchainKHR(instance.vkDevice, instance.vkSwapChain, nullptr);
    vkDestroyImageView(instance.vkDevice, instance.vkDepthImageView, nullptr);
    vkDestroyImage(instance.vkDevice, instance.vkDepthImage, nullptr);
    vkFreeMemory(instance.vkDevice, instance.vkDepthImageMemory, nullptr);

    // Destroy descriptor pool
    vkDestroyDescriptorPool(instance.vkDevice, instance.vkDescriptorPool, nullptr);

    // Destroy surface
    vkDestroySurfaceKHR(instance.vkInstance, instance.vkSurface, nullptr);

    // Destroy devices
    instance.VRAMAllocator.FreeAllPools(&instance.vkDevice);
    vkDestroyDevice(instance.vkDevice, nullptr);

    // Destroy instance
    vkDestroyInstance(instance.vkInstance, nullptr);
}

