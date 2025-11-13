#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"
#include "BufferStructs.h"

// Include ASSIMP headers, (Kulling and assimp team, 2025) v6.0.2
#include <assimp/Importer.hpp>    // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

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

	void LoadFile(std::string path, int meshIndex = -1);
	void LoadFile(const aiScene* scene, int meshIndex = -1);
	static std::vector<TriListMesh> LoadMultiMeshFile(std::string path, bool copyAllToVRAM = true);

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

