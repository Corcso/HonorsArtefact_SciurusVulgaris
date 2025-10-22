#pragma once
#include "VulkanMemoryAllocator.h"

class PointMesh
{
public:
	struct Point {
		HMM_Vec3 position;
		HMM_Vec3 color;
	};

	std::vector<Point> points;
	std::vector<uint32_t> indices;
	void CopyPointsToVRAM();

	// Vulkan Buffers
	VkBuffer pointBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock pointBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;
};

