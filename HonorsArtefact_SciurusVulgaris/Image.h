#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"

class Image
{
public:
	Image();

	void CreateImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage);
	void CreateImageView(bool isDepth = false);
	void DebugNameImage(std::string name); 

	bool CreateAndLoadImageFromFile(std::string path, VkImageUsageFlags usage);
	bool LoadImageFromFile(std::string path);

	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

	VkImageView GetImageView() const { if (hasImageView) return vkImageView; return VK_NULL_HANDLE; }
	VkImage GetImage() const { if (hasImage) return vkImage; return VK_NULL_HANDLE; }
	VkFormat GetImageFormat() const { if (hasImage) return vkFormat; return VK_FORMAT_UNDEFINED; }
	VkExtent2D GetImageExtent() const { if (hasImage) return vkExtent; return {0, 0}; }

	std::unique_ptr<std::vector<uint8_t>> ExtractImageData();

	void Destroy();
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

