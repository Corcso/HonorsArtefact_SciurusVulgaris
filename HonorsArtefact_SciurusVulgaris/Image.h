#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"

/// <summary>
/// Image class, a convienent wrapper around VkImage which stores its own memory, format, extent and such
/// </summary>
class Image
{
public:
	Image();

	/// <summary>
	/// Create a new image on the GPU
	/// </summary>
	/// <param name="format">Image Format</param>
	/// <param name="width">Width</param>
	/// <param name="height">Height</param>
	/// <param name="usage">Image Usage Flags</param>
	void CreateImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage);
	/// <summary>
	/// Create image view, image must already exist
	/// </summary>
	/// <param name="isDepth">If the image stores depth data</param>
	void CreateImageView(bool isDepth = false);
	/// <summary>
	/// Set the images debug name.
	/// </summary>
	void DebugNameImage(std::string name); 

	/// <summary>
	/// Loads and image from file, and creates it. No need to call CreateImage
	/// </summary>
	/// <param name="path">Path of file</param>
	/// <param name="usage">Usage Flags</param>
	/// <returns>Success</returns>
	bool CreateAndLoadImageFromFile(std::string path, VkImageUsageFlags usage);

	/// <summary>
	/// Transitions the image layout, transition must be supported by VulkanUtility
	/// </summary>
	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	// Getters for image components

	VkImageView GetImageView() const { if (hasImageView) return vkImageView; return VK_NULL_HANDLE; }
	VkImage GetImage() const { if (hasImage) return vkImage; return VK_NULL_HANDLE; }
	VkFormat GetImageFormat() const { if (hasImage) return vkFormat; return VK_FORMAT_UNDEFINED; }
	VkExtent2D GetImageExtent() const { if (hasImage) return vkExtent; return {0, 0}; }

	/// <summary>
	/// Copy the raw image bytes back to the CPU.
	/// </summary>
	/// <returns>Unique Pointer to a vector of bytes.</returns>
	std::unique_ptr<std::vector<uint8_t>> ExtractImageData();

	/// <summary>
	/// Destroy the images components
	/// </summary>
	void Destroy();

	/// <summary>
	/// Calls Destroy if needed. 
	/// </summary>
	~Image();
private:
	bool hasImage;
	VkDeviceSize imageSize;
	VkFormat vkFormat;
	VkExtent2D vkExtent;
	VkImage vkImage;
	VulkanMemoryAllocator::VulkanMemoryBlock imageMemory;
	bool hasImageView;
	VkImageView vkImageView;
};

