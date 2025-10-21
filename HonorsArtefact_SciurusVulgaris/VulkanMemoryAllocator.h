#pragma once
#include "PCH.h"
#include <unordered_map>
#include <map>

	class VulkanMemoryAllocator
	{
	public:
		enum class VulkanMemoryMapUsage {
			NONE, INSTANT, OPEN

		};

		struct VulkanMemoryPoolIdentifier {
			uint32_t memoryTypeIndex;
			uint32_t blockSize;
			VulkanMemoryMapUsage mapUsage;

			bool operator==(const VulkanMemoryPoolIdentifier& other) const
			{
				return (memoryTypeIndex == other.memoryTypeIndex
					&& blockSize == other.blockSize
					&& mapUsage == other.mapUsage);
			}

			// Custom hash function for use in Map https://stackoverflow.com/questions/17016175/c-unordered-map-using-a-custom-class-type-as-the-key
			struct Hash {
				std::size_t operator()(const VulkanMemoryAllocator::VulkanMemoryPoolIdentifier& k) const noexcept
				{

					// Compute individual hash values for first,
					// second and third and combine them using XOR
					// and bit shifting:

					return ((std::hash<uint32_t>()(k.memoryTypeIndex)
						^ (std::hash<uint32_t>()(k.blockSize) << 1)) >> 1)
						^ (std::hash<int>()((int)k.mapUsage) << 1);
				}
			};
		};
		
		struct VulkanMemoryPoolLocation {
			uint32_t poolIndex;
			uint32_t offset;
			void* openMap = nullptr;
		};
		struct VulkanMemoryBlock {
			VulkanMemoryPoolIdentifier poolID;
			VulkanMemoryPoolLocation location;
		};

		VulkanMemoryBlock BindBufferToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkBuffer toBind);
		VulkanMemoryBlock BindImageToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkImage toBind);
		void FreeMemory(VkDevice device, VulkanMemoryBlock block);
		/// <summary>
		/// Returns a blocks memory allocation, please note you still need to use the offset. DO NOT free this!
		/// </summary>
		/// <param name="block">Block</param>
		/// <returns>Block's memory allocation</returns>
		VkDeviceMemory GetBlockMemoryAllocation(VulkanMemoryAllocator::VulkanMemoryBlock block);

		void RenderMemoryUsageStat();
	private:
		

		std::unordered_map<VulkanMemoryPoolIdentifier, std::vector<VkDeviceMemory>, VulkanMemoryPoolIdentifier::Hash> memoryPools;
		
		std::unordered_map<VulkanMemoryPoolIdentifier, std::list<VulkanMemoryPoolLocation>, VulkanMemoryPoolIdentifier::Hash> freeMemoryLocations;

		VulkanMemoryBlock FindMemoryBlock(VkDevice device, VkMemoryAllocateInfo desiredAllocation, VulkanMemoryMapUsage mapUsage);

		std::map<uint32_t, uint32_t> sizeToBlockCountPerAlloc = {
			{128, 64},
			{256, 64},
			{512, 64},
			{1024, 64},
			{2048, 64}
		};
	};

template <>
struct std::hash<VulkanMemoryAllocator::VulkanMemoryPoolIdentifier>
{
	
};

