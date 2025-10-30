#pragma once
#include "VulkanMemoryAllocator.h"

class TriListMesh
{
public:
	struct Vertex {
		HMM_Vec3 position;
		HMM_Vec3 color;
	};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	void CopyPointsToVRAM();

	void LoadFile(std::string path);

	// Vulkan Buffers
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock vertexBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;
};

