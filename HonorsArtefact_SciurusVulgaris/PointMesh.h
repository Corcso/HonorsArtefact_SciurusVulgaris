#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"

/// <summary>
/// Point mesh. Stores point based meshes. 
/// </summary>
class PointMesh
{
	// Fully public for ease of development and access. No variables are sensitive.
public:
	/// <summary>
	/// 1 Single Point
	/// </summary>
	struct Point {
		HMM_Vec3 position;
		HMM_Vec3 color;
		HMM_Vec3 normal;
	};

	/// <summary>
	/// 1 Single Point, padded for use within storage buffers
	/// </summary>
	struct PointPadded {
		HMM_Vec3 position; float p_0;
		HMM_Vec3 color; float p_1;
		HMM_Vec3 normal; float p_2;
	};

	std::vector<Point> points;
	/// <summary>
	/// Copy points stored in the points array to the GPU. In a vertex buffer. 
	/// </summary>
	void CopyPointsToVRAM();
	/// <summary>
	/// Copy points stored in the points array to the GPU. In a storage buffer
	/// </summary>
	/// <param name="bindingIndex">Index of storage buffer</param>
	void CopyPointsToVRAMMeshBuffer(uint32_t bindingIndex);

	/// <summary>
	/// Get byte size of point array. 
	/// </summary>
	/// <param name="padded">If the array is padded or not</param>
	/// <returns>Byte size of all points</returns>
	size_t GetPointsArraySize(bool padded = false) { 
		if (padded) return sizeof(PointPadded) * points.size();
		else return sizeof(points[0]) * points.size(); 
	}

	/// <summary>
	/// Returns the meshlet count. Meshlets are fixed at 128 vertices per.
	/// </summary>
	uint32_t GetMeshletCount() { return ceil(points.size() / 128.0f); }

	/// <summary>
	/// Old load from obj file. Not recomended as it is slow and doesn't use assimp (Kulling and assimp team, 2025)
	/// </summary>
	/// <param name="pathOBJ">OBJ Path</param>
	/// <param name="pathMTL">MTL Path</param>
	void LoadFromFileOBJMTL(std::string pathOBJ, std::string pathMTL);

	/// <summary>
	/// Use assimp to load a point cloud.
	/// </summary>
	void LoadFromFile(std::string path);

	/// <summary>
	/// Save the point cloud to file. FBX
	/// </summary>
	void SaveToFile(std::string path);

	// Descriptor and shadow desciptor set setup & getters. 

	void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

	VulkanObjectDescriptorSet* GetDescriptorSet() { return &descriptor; };

	void CreateShadowDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

	VulkanObjectDescriptorSet* GetShadowDescriptorSet() { return &shadowDescriptor; };

	// Vulkan Buffers
	bool isDataOnGPU = false;
	VkBuffer pointBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock pointBufferMemory;

	VulkanObjectDescriptorSet descriptor;
	VulkanObjectDescriptorSet shadowDescriptor;

	/// <summary>
	/// Deconstructor destroys vulkan resources. 
	/// </summary>
	~PointMesh();
};

