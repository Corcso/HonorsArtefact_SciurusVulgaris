#include "PCH.h"

#include "VulkanUtility.h"
#include "Graphics.h"

void VulkanUtility::DebugNameObject(std::string name, uint64_t handle, VkObjectType type)
{
    VkDebugUtilsObjectNameInfoEXT info;
    info.objectHandle = handle;
    info.objectType = type;
    info.pObjectName = name.c_str();
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    info.pNext = nullptr;
    ((PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(Graphics::GetVkDevice(), "vkSetDebugUtilsObjectNameEXT"))(Graphics::GetVkDevice(), &info);
}

VkShaderModule VulkanUtility::CreateShaderModule(VkDevice device, const std::vector<char>& code)
{
    // Struct to create shader module
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());// Vectors store with big alignments(so this is actuall ok 1->4 byte cast)

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw -1;
    }

    return shaderModule;
}

uint32_t VulkanUtility::FindMemoryTypeIndex(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    // Query available types of memory
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    // Find memory type which is suitable
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw -1;
}

//void VulkanUtility::CreateBufferAndAssignMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* bufferMemory)
void VulkanUtility::CreateBufferAndAssignMemory(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VulkanMemoryAllocator::VulkanMemoryBlock* bufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage)
{
    

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(Graphics::GetVkDevice(), &bufferInfo, nullptr, buffer) != VK_SUCCESS) {
        throw -1;
    }

    /*VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Graphics::GetVkDevice(), *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryTypeIndex(graphicsService->physicalDevice, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(Graphics::GetVkDevice(), &allocInfo, nullptr, bufferMemory) != VK_SUCCESS) {
        throw -1;
    }*/

    *bufferMemory = Graphics::GetMemoryAllocator().BindBufferToMemory(Graphics::GetVkDevice(), Graphics::GetVkPhysicalDevice(), properties, mapUsage, *buffer);

//    vkBindBufferMemory(Graphics::GetVkDevice(), *buffer, *bufferMemory, 0);
}

void VulkanUtility::CreateImageAndAssignMemory(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage* image, VulkanMemoryAllocator::VulkanMemoryBlock* imageMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage mapUsage)
{
    

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(Graphics::GetVkDevice(), &imageInfo, nullptr, image) != VK_SUCCESS) {
        throw -1;
    }

    *imageMemory = Graphics::GetMemoryAllocator().BindImageToMemory(Graphics::GetVkDevice(), Graphics::GetVkPhysicalDevice(), properties, mapUsage, *image);
}

void VulkanUtility::DestroyBuffer(VkBuffer buffer)
{
    
    
    vkDestroyBuffer(Graphics::GetVkDevice(), buffer, nullptr);
    

}

void VulkanUtility::DestroyImage(VkImage image)
{
    

    vkDestroyImage(Graphics::GetVkDevice(), image, nullptr);
}

void VulkanUtility::FreeGPUMemory(VkDeviceMemory memory)
{
    

    vkFreeMemory(Graphics::GetVkDevice(), memory, nullptr);
}

void VulkanUtility::FreeGPUMemoryBlock(VulkanMemoryAllocator::VulkanMemoryBlock memoryBlock)
{
    
    Graphics::GetMemoryAllocator().FreeMemory(Graphics::GetVkDevice(), memoryBlock);
}

void VulkanUtility::CopyBufferData(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = Graphics::GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Graphics::GetVkDevice(), &allocInfo, &commandBuffer);

    // Start recording now
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    // Execute commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Graphics::GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Graphics::GetVkGraphicsQueue());

    vkFreeCommandBuffers(Graphics::GetVkDevice(), Graphics::GetCommandPool(), 1, &commandBuffer);
}

void VulkanUtility::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = Graphics::GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Graphics::GetVkDevice(), &allocInfo, &commandBuffer);

    // Start recording now
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    // Do stuff

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    // Over
    vkEndCommandBuffer(commandBuffer);

    // Execute commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Graphics::GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Graphics::GetVkGraphicsQueue());

    vkFreeCommandBuffers(Graphics::GetVkDevice(), Graphics::GetCommandPool(), 1, &commandBuffer);
}

void VulkanUtility::CopyImageToBuffer(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = Graphics::GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Graphics::GetVkDevice(), &allocInfo, &commandBuffer);

    // Start recording now
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    // Do stuff

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyImageToBuffer(
        commandBuffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        buffer,
        1,
        &region
    );

    // Over
    vkEndCommandBuffer(commandBuffer);

    // Execute commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Graphics::GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Graphics::GetVkGraphicsQueue());

    vkFreeCommandBuffers(Graphics::GetVkDevice(), Graphics::GetCommandPool(), 1, &commandBuffer);
}

void VulkanUtility::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth)
{
    

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = Graphics::GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(Graphics::GetVkDevice(), &allocInfo, &commandBuffer);

    // Start recording now
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    
    // Do stuff
    TransitionImageLayout(commandBuffer, image, format, oldLayout, newLayout, isDepth);

    // Over
    vkEndCommandBuffer(commandBuffer);

    // Execute commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(Graphics::GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(Graphics::GetVkGraphicsQueue());

    vkFreeCommandBuffers(Graphics::GetVkDevice(), Graphics::GetCommandPool(), 1, &commandBuffer);
}

void VulkanUtility::TransitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, bool isDepth)
{
    // Barriers can also be used to change image format for some reason.
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = !isDepth ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = 0;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    }
    else {
        throw - 1; // Unsupported layout transition
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void VulkanUtility::MapCopyToGPU(VkDeviceMemory memory, void* data, size_t size, VkDeviceSize offset, VkMemoryMapFlags flags)
{
    
    void* mappedMemory;
    vkMapMemory(Graphics::GetVkDevice(), memory, offset, size, flags, &mappedMemory);
    // Copy Data
    memcpy(mappedMemory, data, size);
    // Unmap Data
    vkUnmapMemory(Graphics::GetVkDevice(), memory);
}

void VulkanUtility::MapCopyBlockToGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags)
{
    
    if (memory.poolID.mapUsage == VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN) return;
    void* mappedMemory;
    vkMapMemory(Graphics::GetVkDevice(), Graphics::GetMemoryAllocator().GetBlockMemoryAllocation(memory), memory.location.offset, size, flags, &mappedMemory);
    // Copy Data
    memcpy(mappedMemory, data, size);
    // Unmap Data
    vkUnmapMemory(Graphics::GetVkDevice(), Graphics::GetMemoryAllocator().GetBlockMemoryAllocation(memory));
}

void VulkanUtility::MapCopyBlockFromGPU(VulkanMemoryAllocator::VulkanMemoryBlock memory, void* data, size_t size, VkMemoryMapFlags flags)
{

    if (memory.poolID.mapUsage == VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN) return;
    void* mappedMemory;
    vkMapMemory(Graphics::GetVkDevice(), Graphics::GetMemoryAllocator().GetBlockMemoryAllocation(memory), memory.location.offset, size, flags, &mappedMemory);
    // Copy Data
    memcpy(data, mappedMemory, size);
    // Unmap Data
    vkUnmapMemory(Graphics::GetVkDevice(), Graphics::GetMemoryAllocator().GetBlockMemoryAllocation(memory));
}

void* VulkanUtility::OpenMemoryMap(VkDeviceMemory memory, size_t size, VkDeviceSize offset, VkMemoryMapFlags flags)
{
    
    void* toReturn;
    vkMapMemory(Graphics::GetVkDevice(), memory, offset, size, flags, &toReturn);
    return toReturn;
}
void* VulkanUtility::OpenMemoryBlockMap(VulkanMemoryAllocator::VulkanMemoryBlock memory, size_t size, VkMemoryMapFlags flags)
{
    /*if (memory.poolID.instantCloseMap) return nullptr;
    
    void* toReturn;
    vkMapMemory(Graphics::GetVkDevice(), Graphics::GetMemoryAllocator().GetBlockMemoryAllocation(memory), memory.location.offset, size, flags, &toReturn);
    return toReturn;*/
    return nullptr;
}