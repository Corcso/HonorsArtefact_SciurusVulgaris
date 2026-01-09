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
	void CopyPointsToVRAM();

	void LoadFromFileOBJMTL(std::string pathOBJ, std::string pathMTL);
	void LoadFromFile(std::string path);
	void SaveToFile(std::string path);

	void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

	VulkanObjectDescriptorSet* GetDescriptorSet() { return &descriptor; };

	// Vulkan Buffers
	bool isDataOnGPU = false;
	VkBuffer pointBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock pointBufferMemory;

	VulkanObjectDescriptorSet descriptor;

	~PointMesh();
};

