#include "PCH.h"
#include "PointMesh.h"
#include "VulkanUtility.h"

void PointMesh::CopyPointsToVRAM()
{
    // VERTEX BUFFER
    
    // Staging Vertex buffer
    VkBuffer stagingVertexBuffer;
    //VkDeviceMemory stagingVertexBufferMemory;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingVertexBufferMemory;

    VkDeviceSize bufferSize = sizeof(points[0]) * points.size();
    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingVertexBuffer, &stagingVertexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    // Map GPU memory to CPU memory
    VulkanUtility::MapCopyBlockToGPU(stagingVertexBufferMemory, points.data(), bufferSize);

    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &pointBuffer, &pointBufferMemory);

    // See below
    VulkanUtility::CopyBufferData(stagingVertexBuffer, pointBuffer, bufferSize);

    // Dont need the stager anymore
    VulkanUtility::DestroyBuffer(stagingVertexBuffer);
    //VulkanUtility::FreeGPUMemory(stagingVertexBufferMemory);
    VulkanUtility::FreeGPUMemoryBlock(stagingVertexBufferMemory);

    // INDEX BUFFER
    bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingIndexBuffer;
    VulkanMemoryAllocator::VulkanMemoryBlock stagingIndexBufferMemory;
    //VkDeviceMemory stagingIndexBufferMemory;
    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &stagingIndexBuffer, &stagingIndexBufferMemory, VulkanMemoryAllocator::VulkanMemoryMapUsage::INSTANT);

    VulkanUtility::MapCopyBlockToGPU(stagingIndexBufferMemory, indices.data(), bufferSize);

    VulkanUtility::CreateBufferAndAssignMemory(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &indexBuffer, &indexBufferMemory);

    VulkanUtility::CopyBufferData(stagingIndexBuffer, indexBuffer, bufferSize);

    VulkanUtility::DestroyBuffer(stagingIndexBuffer);
    VulkanUtility::FreeGPUMemoryBlock(stagingIndexBufferMemory);
    //VulkanUtility::FreeGPUMemory(stagingIndexBufferMemory);
}
