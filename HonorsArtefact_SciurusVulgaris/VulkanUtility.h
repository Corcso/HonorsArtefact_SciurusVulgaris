#pragma once

#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include <fstream>

/// <summary>
/// Static Utility class to provide common vulkan functions. 
/// Contains some code from (Overvoorde, 2023)
/// </summary>
class VulkanUtility
{
public:
    /// <summary>
    /// Name an object. 
    /// </summary>
    /// <param name="name">Name</param>
    /// <param name="handle">Object Handle</param>
    /// <param name="type">Type Idenitfier</param>
    static void DebugNameObject(std::string name, uint64_t handle, VkObjectType type);

	/// <summary>
	/// Create a shader module
	/// </summary>
	/// <param name="device">Device</param>
	/// <param name="code">Shader Code</param>
	/// <returns>The Shader Module</returns>
	static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

    /// <summary>
    /// Find memory type index based on properties and filter. 
    /// </summary>
    /// <param name="physicalDevice">Physical Device</param>
    /// <param name="typeFilter">Type Filter</param>
    /// <param name="properties">Properties</param>
    /// <returns>Memory Type Index</returns>
    static uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties); 

    /// <summary>
    /// Create buffer and assign memory
    /// </summary>
    /// <param name="size">Size of buffer</param>
    /// <param name="usage">Buffer usage</param>
    /// <param name="properties">Buffer properties</param>
    /// <param name="buffer">OUT buffer</param>
    /// <param name="bufferMemory">OUT buffer memory block</param>
    /// <param name="mapUsage">Map usage for buffer, auto to none</param>
    static void CreateBufferAndAssignMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VulkanMemoryAllocator::VulkanMemoryBlock* bufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage = VulkanMemoryAllocator::VulkanMemoryMapUsage::NONE);
    /// <summary>
    /// Create an image and assign memory
    /// </summary>
    /// <param name="image">OUT Image</param>
    /// <param name="imageMemory">OUT Image Memory Block</param>
    /// <param name="mapUsage">Map usage for buffer, auto to none</param>
    static void CreateImageAndAssignMemory(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VulkanMemoryAllocator::VulkanMemoryBlock* imageMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage = VulkanMemoryAllocator::VulkanMemoryMapUsage::NONE);
    /// <summary>
    /// Destroys a buffer, does not free its memory.
    /// </summary>
    static void DestroyBuffer(VkBuffer buffer);
    /// <summary>
    /// Destroys an image, does not free its memory.
    /// </summary>
    static void DestroyImage(VkImage image);
    /// <summary>
    /// Free GPU memory, using a Raw GPU Memory Handle
    /// </summary>
    static void FreeGPUMemory(VkDeviceMemory memory);
    /// <summary>
    /// Free GPU memory, from the small pools allocator
    /// </summary>
    static void FreeGPUMemoryBlock(VulkanMemoryAllocator::VulkanMemoryBlock memoryBlock);

    /// <summary>
    /// Copy buffer -> buffer
    /// </summary>
    static void CopyBufferData(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    /// <summary>
    /// Copy buffer -> image
    /// </summary>
    static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    /// <summary>
    /// Copy image -> buffer
    /// </summary>
    static void CopyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    /// <summary>
    /// Transition an image layout, transition must be supported. Create a command buffer and execute now
    /// </summary>
    static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth = false);
    /// <summary>
    /// Transition an image layout, transition must be supported. Writes to a given command buffer
    /// </summary>
    static void TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth = false);

    /// <summary>
    /// Open a memory map, copy data, then close the map. CPU -> GPU
    /// </summary>
    static void MapCopyToGPU(VkDeviceMemory memory, void* data, size_t size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0);
    /// <summary>
    /// Open a memory map, copy data, then close the map. CPU -> GPU Only for INSTANT mapping type memory.
    /// </summary>
    static void MapCopyBlockToGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags = 0);
    /// <summary>
    /// Open a memory map, copy data, then close the map. GPU -> CPU Only for INSTANT mapping type memory.
    /// </summary>
    static void MapCopyBlockFromGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags = 0);
    /// <summary>
    /// Opem a memory map
    /// </summary>
    /// <returns>Pointer to mapped CPU memory </returns>
    static void* OpenMemoryMap(VkDeviceMemory memory, size_t size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0);

	/// <summary>
	/// Read raw bytes out of a file, used for shader code reads. 
	/// </summary>
	static inline std::vector<char> ReadFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw -1;
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;

    }
};
