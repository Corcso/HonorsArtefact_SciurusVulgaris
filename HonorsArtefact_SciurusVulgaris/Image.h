#pragma once
#include "PCH.h"
#include "VulkanMemoryAllocator.h"

class Image
{
public:
	Image();

	void CreateImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage);
	void CreateImageView();

	VkImageView GetImageView() const { if (hasImageView) return vkImageView; return VK_NULL_HANDLE; }
	VkImage GetImage() const { if (hasImage) return vkImage; return VK_NULL_HANDLE; }

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

