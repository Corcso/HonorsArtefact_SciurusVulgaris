#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"

class PointMesh
{
public:
	struct Point {
		HMM_Vec3 position;
		HMM_Vec3 color;
		HMM_Vec3 normal;
	};

	std::vector<Point> points;
	std::vector<uint32_t> indices;
	void CopyPointsToVRAM();

	void LoadFromFileOBJMTL(std::string pathOBJ, std::string pathMTL);
	void LoadFromFile(std::string path);

	void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

	VulkanObjectDescriptorSet* GetDescriptorSet() { return &descriptor; };

	// Vulkan Buffers
	VkBuffer pointBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock pointBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;

	VulkanObjectDescriptorSet descriptor;

	~PointMesh();
};

