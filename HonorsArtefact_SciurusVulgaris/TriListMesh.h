#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"
#include "BufferStructs.h"

class TriListMesh
{
public:
	struct Vertex {
		HMM_Vec3 position;
		HMM_Vec3 normal;
		HMM_Vec2 textureCoordinate;
	};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	void CopyPointsToVRAM();

	void LoadFile(std::string path);

	void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);
	VulkanObjectDescriptorSet* GetDescriptorSet() { return &descriptor; };

	// Vulkan Buffers
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock vertexBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;

	VulkanObjectDescriptorSet descriptor;

	~TriListMesh();
};

