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

	struct PointPadded {
		HMM_Vec3 position; float p_0;
		HMM_Vec3 color; float p_1;
		HMM_Vec3 normal; float p_2;
	};

	std::vector<Point> points;
	void CopyPointsToVRAM();
	void CopyPointsToVRAMMeshBuffer(uint32_t bindingIndex);
	size_t GetPointsArraySize(bool padded = false) { 
		if (padded) return sizeof(PointPadded) * points.size();
		else return sizeof(points[0]) * points.size(); 
	}
	uint32_t GetMeshletCount() { return ceil(points.size() / 64.0f); }

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

