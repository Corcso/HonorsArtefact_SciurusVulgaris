#pragma once

// STL Includes
#include <iostream>
#include <string>
#include <cstdint>

// Windows Includes
#include <windows.h>

// Vulkan Includes
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

// Other
#include "HandmadeMath.h"