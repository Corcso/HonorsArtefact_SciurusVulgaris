#include "PCH.h"
#include "Image.h"
#include "Graphics.h"
#include "VulkanUtility.h"
#include "VulkanSetup.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

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

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(Graphics::GetVkDevice(), vkImage, &memRequirements);
    imageSize = memRequirements.size;
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

void Image::DebugNameImage(std::string name)
{
    if (hasImage) VulkanUtility::DebugNameObject(name + " Image", (uint64_t)vkImage, VK_OBJECT_TYPE_IMAGE);
    if (hasImageView) VulkanUtility::DebugNameObject(name + " Image", (uint64_t)vkImageView, VK_OBJECT_TYPE_IMAGE_VIEW);
}

bool Image::CreateAndLoadImageFromFile(std::string path, VkImageUsageFlags usage)
{
    // (Overvoorde, no date) https://vulkan-tutorial.com/Texture_mapping/Images
    // Load image
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    imageSize = texWidth * texHeight * 4;

    VkBuffer stagingBuffer;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingBufferMemory;

    VulkanUtility::CreateBufferAndAssignMemory(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    // Copy image into staging buffer
    VulkanUtility::MapCopyBlockToGPU(stagingBufferMemory, pixels, imageSize);

    // Create image, with implicit transfer destination usage
    CreateImage(VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

    // Transition image layout
    VulkanUtility::TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // Copy data
    VulkanUtility::CopyBufferToImage(stagingBuffer, vkImage, texWidth, texHeight);
    // Transition to shader read (how to make more generic)
    VulkanUtility::TransitionImageLayout(vkImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Cleanup
    vkDestroyBuffer(Graphics::GetVkDevice(), stagingBuffer, nullptr);
    VulkanUtility::FreeGPUMemoryBlock(stagingBufferMemory);

    stbi_image_free(pixels);

    return true;
}

bool Image::LoadImageFromFile(std::string path)
{
    return false;
}

void Image::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VulkanUtility::TransitionImageLayout(vkImage, vkFormat, oldLayout, newLayout, vkFormat == VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()));
}

std::unique_ptr<std::vector<uint8_t>> Image::ExtractImageData()
{
    VkBuffer stagingBuffer;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingBufferMemory;

    VulkanUtility::CreateBufferAndAssignMemory(imageSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingBuffer, &stagingBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    VulkanUtility::TransitionImageLayout(vkImage, vkFormat, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

    VulkanUtility::CopyImageToBuffer(stagingBuffer, vkImage, vkExtent.width, vkExtent.height);

    VulkanUtility::TransitionImageLayout(vkImage, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    std::unique_ptr<std::vector<uint8_t>> data = std::make_unique<std::vector<uint8_t>>(imageSize);
    VulkanUtility::MapCopyBlockFromGPU(stagingBufferMemory, data->data(), data->size());

    // Cleanup
    vkDestroyBuffer(Graphics::GetVkDevice(), stagingBuffer, nullptr);
    VulkanUtility::FreeGPUMemoryBlock(stagingBufferMemory);

    return std::move(data);
}

void Image::Destroy()
{
    if (hasImageView) vkDestroyImageView(Graphics::GetVkDevice(), vkImageView, nullptr);
    if (hasImage) vkDestroyImage(Graphics::GetVkDevice(), vkImage, nullptr);
    hasImageView = false;
    hasImage = false;
}

Image::~Image()
{
    Destroy();
}
