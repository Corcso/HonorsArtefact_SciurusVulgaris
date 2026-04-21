#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"
#include "BufferStructs.h"

// Include ASSIMP headers, (Kulling and assimp team, 2025) v6.0.2
#include <assimp/Importer.hpp>    // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

/// <summary>
/// Triangle Mesh storage class.
/// </summary>
class TriListMesh
{
public:
	/// <summary>
	/// Vertex representation
	/// </summary>
	struct Vertex {
		HMM_Vec3 position;
		HMM_Vec3 normal;
		HMM_Vec2 textureCoordinate;
	};
	// No padded version as triangle meshes are not used within mesh shaders. 

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	/// <summary>
	/// Copy vertices stored in the points array to the GPU. In a vertex & index buffer. 
	/// </summary>
	void CopyPointsToVRAM();
	
	/// <summary>
	/// Loads file from path, or returns the chosen mesh index. 
	/// </summary>
	void LoadFile(std::string path, int meshIndex);
	/// <summary>
	/// Loads file from assimp scene, returns the chosen mesh index. 
	/// </summary>
	void LoadFile(const aiScene* scene, int meshIndex);

	/// <summary>
	/// Static helper function which will load a file but split per mesh. 
	/// Each mesh is stored as an entry within a vector. 
	/// </summary>
	/// <param name="copyAllToVRAM">Used to quickly copy all meshes to GPU if desired.</param>
	/// <returns>A vector containing all triangle meshes.</returns>
	static std::vector<TriListMesh> LoadMultiMeshFile(std::string path, bool copyAllToVRAM = true);

	// Descriptor set creator and getter

	void CreateDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);
	VulkanObjectDescriptorSet* GetDescriptorSet() { return &descriptor; };

	// Vulkan Buffers
	VkBuffer vertexBuffer;
	VkBuffer indexBuffer;
	VulkanMemoryAllocator::VulkanMemoryBlock vertexBufferMemory;
	VulkanMemoryAllocator::VulkanMemoryBlock indexBufferMemory;

	VulkanObjectDescriptorSet descriptor;

	/// <summary>
	/// All vulkan objects destroyed on deconstruction. 
	/// </summary>
	~TriListMesh();
};

