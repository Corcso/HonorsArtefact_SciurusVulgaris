#pragma once

// STL Includes
#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>

// Windows Includes
#include <windows.h>

// Vulkan Includes
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

// Other
#include "HandmadeMath.h"
#include <stb/stb_image.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_vulkan.h"

#ifdef NV_PERF_METER
//#include <NvPerfUtilities.h>
#include <NvPerfVulkan.h>
//#include <NvPerfPeriodicSamplerCommon.h>
#include <NvPerfReportGenerator.h>
#include <NvPerfReportGeneratorVulkan.h>
//#include <NvPerfHudRenderer.h>
#endif // NV_PERF_METER
