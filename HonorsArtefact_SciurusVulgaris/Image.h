#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"

class Image
{
public:
	Image();

	void CreateImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage);
	void CreateImageView(bool isDepth = false);

	VkImageView GetImageView() const { if (hasImageView) return vkImageView; return VK_NULL_HANDLE; }
	VkImage GetImage() const { if (hasImage) return vkImage; return VK_NULL_HANDLE; }
	VkFormat GetImageFormat() const { if (hasImage) return vkFormat; return VK_FORMAT_UNDEFINED; }
	VkExtent2D GetImageExtent() const { if (hasImage) return vkExtent; return {0, 0}; }

	void Destroy();
	~Image();
private:
	bool hasImage;
	VkFormat vkFormat;
	VkExtent2D vkExtent;
	VkImage vkImage;
	VulkanMemoryAllocator::VulkanMemoryBlock imageMemory;
	bool hasImageView;
	VkImageView vkImageView;
};

