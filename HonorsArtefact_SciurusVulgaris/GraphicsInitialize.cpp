#include "PCH.h"
#include "Graphics.h"
#include "VulkanUtility.h"
#include "VulkanSetup.h"
#include "Input.h"

// The setup process is so long its getting its own CPP file.

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

    // Setup app info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "2200592-SciurusVulgaris";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Setup instance creation info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // We are having 2 extensions, windows and surface
    std::vector<const char*> extensionsNames = {
        "VK_KHR_surface", "VK_KHR_win32_surface"
    };

    createInfo.enabledExtensionCount = extensionsNames.size();
    createInfo.ppEnabledExtensionNames = extensionsNames.data();

    // Validation layers layeers
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    // Create the instance
    if (vkCreateInstance(&createInfo, nullptr, &instance.vkInstance) != VK_SUCCESS) {
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

    std::cout << "Enter Chosen Index: ";
    std::string choice;
    std::cin >> choice;
    if (std::stoi(choice) >= deviceCount) return; // Fail!

    instance.vkPhysicalDevice = devices[std::stoi(choice)];

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

    // Coming back later here
    VkPhysicalDeviceFeatures deviceFeatures{};

    // Device info
    VkDeviceCreateInfo logicDeviceCreateInfo{};
    logicDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    logicDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    logicDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    logicDeviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    logicDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(VK_DEVICE_EXTENSIONS_REQUIRED.size());
    std::vector<const char*> deviceExtensionsAsCStr;
    for (const auto& string : VK_DEVICE_EXTENSIONS_REQUIRED) deviceExtensionsAsCStr.push_back(string.c_str());
    logicDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsAsCStr.data();

    // Not required only for backwards compat
    /*if (enableValidationLayers) {
        logicDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        logicDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        logicDeviceCreateInfo.enabledLayerCount = 0;
    }*/

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
    VulkanSetup::CreateDescriptorPool(instance.vkDevice, 1, VULKAN_MAX_FRAMES_IN_FLIGHT * 100, &instance.vkDescriptorPool);

    std::vector<VkDescriptorSetLayoutBinding> uboLayoutBindings(1);
    uboLayoutBindings[0].binding = 0;
    uboLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBindings[0].descriptorCount = 1;
    // Only using this in vertex shader
    uboLayoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    // Not used for images
    uboLayoutBindings[0].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = uboLayoutBindings.size();
    layoutInfo.pBindings = uboLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(instance.vkDevice, &layoutInfo, nullptr, &instance.vkDescriptorSetLayout) != VK_SUCCESS) {
        throw - 1;
    }

    for (int i = 0; i < VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
        instance.perFramePerObjectDescriptors.push_back(std::vector<VulkanDescriptor>());
    }

    // Setup pipeline
    std::vector<VkDescriptorSetLayout> allDescriptorSetLayouts = {
        instance.vkDescriptorSetLayout
    };
    VulkanSetup::CreateGraphicsPipeline(instance.vkDevice, instance.vkRenderPass, instance.vkSwapChainExtent,
        allDescriptorSetLayouts, &instance.vkMainPipelineLayout, &instance.vkMainPipeline);

    // Setup depth buffer
    VulkanSetup::CreateDepthBuffer(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSwapChainExtent, &instance.vkDepthImage,
        &instance.vkDepthImageMemory, &instance.vkDepthImageView);

    // Setup frame buffers
    VulkanSetup::CreateFrameBuffers(instance.vkDevice, instance.vkRenderPass, instance.vkSwapChainExtent, instance.vkSwapChainImageViews, instance.vkDepthImageView, &instance.vkSwapChainFrameBuffers);

    // Setup command pool & buffers
    VulkanSetup::CreateCommandPool(instance.vkDevice, instance.vkPhysicalDevice, instance.vkSurface, &instance.vkCommandPool);
    VulkanSetup::CreateCommandBuffers(instance.vkDevice, instance.vkCommandPool, &instance.vkCommandBuffers);

    // Create Sync objects
    VulkanSetup::CreateSyncObjects(instance.vkDevice, &instance.vkInFlightFences, &instance.vkImageAvailableSemaphores, &instance.vkRenderFinishedSemaphores);

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
//
//    editorViewportExtent = { 800, 800 };
//
//    VulkanSetup::CreateEditorViewport(device, physicalDevice, editorViewportExtent, swapChainImageFormat, renderPass,
//        &editorViewport, &editorViewportImageView, &editorViewportFrameBuffer, &editorViewportSampler, &editorViewportMemory,
//        &editorDepthImage, &editorDepthImageView, &editorDepthImageMemory);
//
//    editorViewportDescriptorSet = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(editorViewportSampler, editorViewportImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    instance.meshRenderer.CreateAll();

    instance.meshRenderOutput = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(instance.meshRenderer.GetSampler() , instance.meshRenderer.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

    instance.myMesh.LoadFile("./models/Low Poly Trees Free - Nicholas-3D/TreeOne.obj");
    instance.myMesh.CopyPointsToVRAM();

    return ;
}

