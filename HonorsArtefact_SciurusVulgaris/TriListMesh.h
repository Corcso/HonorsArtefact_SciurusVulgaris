#pragma once
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

	// Vulkan Buffers
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock vertexBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;
};

