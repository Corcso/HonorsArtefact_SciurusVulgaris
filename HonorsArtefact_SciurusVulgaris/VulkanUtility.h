#pragma once

#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include <fstream>

	class VulkanUtility
	{
	public:
		static VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

        static uint32_t FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties); 

        //static void CreateBufferAndAssignMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory);
        static void CreateBufferAndAssignMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VulkanMemoryAllocator::VulkanMemoryBlock* bufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage = VulkanMemoryAllocator::VulkanMemoryMapUsage::NONE);
        static void CreateImageAndAssignMemory(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VulkanMemoryAllocator::VulkanMemoryBlock* imageMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage = VulkanMemoryAllocator::VulkanMemoryMapUsage::NONE);
        static void DestroyBuffer(VkBuffer buffer);
        static void DestroyImage(VkImage image);
        static void FreeGPUMemory(VkDeviceMemory memory);
        static void FreeGPUMemoryBlock(VulkanMemoryAllocator::VulkanMemoryBlock memoryBlock);

        static void CopyBufferData(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        static void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static void CopyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        static void MapCopyToGPU(VkDeviceMemory memory, void* data, size_t size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0);
        static void MapCopyBlockToGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags = 0);
        static void MapCopyBlockFromGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags = 0);
        static void* OpenMemoryMap(VkDeviceMemory memory, size_t size, VkDeviceSize offset = 0, VkMemoryMapFlags flags = 0);
        static void* OpenMemoryBlockMap(VulkanMemoryAllocator::VulkanMemoryBlock memory, size_t size, VkMemoryMapFlags flags = 0);

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
