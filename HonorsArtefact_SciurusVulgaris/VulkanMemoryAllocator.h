#pragma once
#include "PCH.h"
#include <unordered_map>
#include <map>

/// <summary>
/// Custom vulkan memory allocator using small pools allocation system. 
/// </summary>
class VulkanMemoryAllocator
{
public:
	/// <summary>
	/// Types of memory map usage, None = Never mapped, Instant = Map opened, data copied, map closed. Open = Map forever open (for descriptor buffers and such)
	/// </summary>
	enum class VulkanMemoryMapUsage {
		NONE, INSTANT, OPEN
	};

	/// <summary>
	/// Memory pool idenfitier, which is hashable.
	/// </summary>
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

		// (jogojapan, 2013)
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
	
	/// <summary>
	/// Location within a memory pool
	/// </summary>
	struct VulkanMemoryPoolLocation {
		uint32_t poolIndex;
		uint32_t offset;
		void* openMap = nullptr;
	};

	/// <summary>
	/// Combined reference for full memory location id
	/// </summary>
	struct VulkanMemoryBlock {
		VulkanMemoryPoolIdentifier poolID;
		VulkanMemoryPoolLocation location;
	};

	/// <summary>
	/// Binds a buffer to a memory block with the desired properties. Creates a new pool & block if not one big enough. 
	/// </summary>
	/// <param name="device">Device</param>
	/// <param name="physicalDevice">Physical Device</param>
	/// <param name="properties">Memory Property Flags</param>
	/// <param name="mapUsage">Map Useage</param>
	/// <param name="toBind">Buffer to bind</param>
	/// <returns>Memory Block ID</returns>
	VulkanMemoryBlock BindBufferToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkBuffer toBind);

	/// <summary>
	/// Binds an image to a memory block with the desired properties. Creates a new pool & block if not one big enough. 
	/// </summary>
	/// <param name="device">Device</param>
	/// <param name="physicalDevice">Physical Device</param>
	/// <param name="properties">Memory Property Flags</param>
	/// <param name="mapUsage">Map Useage</param>
	/// <param name="toBind">Image to bind</param>
	/// <returns>Memory Block ID</returns>
	VulkanMemoryBlock BindImageToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkImage toBind);

	/// <summary>
	/// Frees a memory block. (This might not actually free the GPU memory, just the pools block)
	/// </summary>
	/// <param name="device">Device</param>
	/// <param name="block">Block ID</param>
	void FreeMemory(VkDevice device, VulkanMemoryBlock block);

	/// <summary>
	/// Returns a blocks memory allocation, please note you still need to use the offset. DO NOT free this!
	/// </summary>
	/// <param name="block">Block</param>
	/// <returns>Block's memory allocation</returns>
	VkDeviceMemory GetBlockMemoryAllocation(VulkanMemoryAllocator::VulkanMemoryBlock block);

	/// <summary>
	/// Flush an entire mapped block
	/// </summary>
	/// <param name="device">Device</param>
	/// <param name="block">Block ID</param>
	void FlushMappedBlock(VkDevice device, VulkanMemoryAllocator::VulkanMemoryBlock block);

	/// <summary>
	/// Render an ImGui Usage Window
	/// </summary>
	void RenderMemoryUsageStat();

	/// <summary>
	/// Free all memory pools back to the GPU. Should be called at app shutdown.
	/// </summary>
	/// <param name="device">Device</param>
	void FreeAllPools(VkDevice* device);
private:
	

	std::unordered_map<VulkanMemoryPoolIdentifier, std::vector<VkDeviceMemory>, VulkanMemoryPoolIdentifier::Hash> memoryPools;
	
	std::unordered_map<VulkanMemoryPoolIdentifier, std::list<VulkanMemoryPoolLocation>, VulkanMemoryPoolIdentifier::Hash> freeMemoryLocations;

	// Find a free memory block
	VulkanMemoryBlock FindMemoryBlock(VkDevice device, VkMemoryAllocateInfo desiredAllocation, VulkanMemoryMapUsage mapUsage);

	/// <summary>
	/// Pool Block Sizes and Counts to make by default.
	/// </summary>
	std::map<uint32_t, uint32_t> sizeToBlockCountPerAlloc = {
		{128, 64},
		{256, 64},
		{512, 64},
		{1024, 64},
		{2048, 64},
		{(1024 * 1024), 64}, // 1 MB
		{(2048 * 1024), 32} // 2 MB
	};
};

template <>
struct std::hash<VulkanMemoryAllocator::VulkanMemoryPoolIdentifier>
{
	
};

