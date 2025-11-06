#include "PCH.h"
#include "Image.h"
#include "Graphics.h"
#include "VulkanUtility.h"

Image::Image()
{
    hasImage = false;
    hasImageView = false;
}

void Image::CreateImage(VkFormat format, uint32_t width, uint32_t height, VkImageUsageFlags usage)
{
    hasImage = true;

    vkFormat = format;
    vkExtent.width = width;
    vkExtent.height = height;

    VulkanUtility::CreateImageAndAssignMemory(width, height, format,
        VK_IMAGE_TILING_OPTIMAL, usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &vkImage, &imageMemory);
}

void Image::CreateImageView(bool isDepth)
{
    hasImageView = true;

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = vkImage;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = vkFormat;
    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.subresourceRange.aspectMask = (isDepth) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    vkCreateImageView(Graphics::GetVkDevice(), &imageViewCreateInfo, nullptr, &vkImageView);
}

void Image::Destroy()
{
    if(hasImageView) vkDestroyImageView(Graphics::GetVkDevice(), vkImageView, nullptr);
    if(hasImage) vkDestroyImage(Graphics::GetVkDevice(), vkImage, nullptr);
    hasImageView = false;
    hasImage = false;
}

Image::~Image()
{
    Destroy();
}
