#include "PCH.h"

#include "VulkanMemoryAllocator.h"
#include "VulkanUtility.h"

#include <iostream>

    VulkanMemoryAllocator::VulkanMemoryBlock VulkanMemoryAllocator::BindBufferToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkBuffer toBind)
    {
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, toBind, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanUtility::FindMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, properties);

        VulkanMemoryBlock block = FindMemoryBlock(device, allocInfo, mapUsage);

        vkBindBufferMemory(device, toBind, memoryPools[block.poolID][block.location.poolIndex], block.location.offset);

        return block;
    }

    VulkanMemoryAllocator::VulkanMemoryBlock VulkanMemoryAllocator::BindImageToMemory(VkDevice device, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties, VulkanMemoryMapUsage mapUsage, VkImage toBind)
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, toBind, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = VulkanUtility::FindMemoryTypeIndex(physicalDevice, memRequirements.memoryTypeBits, properties);
        
        VulkanMemoryBlock block = FindMemoryBlock(device, allocInfo, mapUsage);

        vkBindImageMemory(device, toBind, memoryPools[block.poolID][block.location.poolIndex], block.location.offset);
        return block;
    }

    void VulkanMemoryAllocator::FreeMemory(VkDevice device, VulkanMemoryBlock block)
    {
        // If our block size is standard treat it differerantly 
        // TEMP
        // TODO drop all empty pools
        if (sizeToBlockCountPerAlloc.find(block.poolID.blockSize) != sizeToBlockCountPerAlloc.end()) {
            // Just mark as free, don't drop any pools. Need to drop a pool when its completely empty but will require a full linked list traversal at the moment I think.
            freeMemoryLocations[block.poolID].push_front(block.location);
        }
        else {
            // Drop the custom size pool
            vkFreeMemory(device, memoryPools[block.poolID][block.location.poolIndex], nullptr);
            memoryPools[block.poolID].erase(memoryPools[block.poolID].begin() + block.location.poolIndex);
        }
    }

    VkDeviceMemory VulkanMemoryAllocator::GetBlockMemoryAllocation(VulkanMemoryAllocator::VulkanMemoryBlock block)
    {
        return memoryPools[block.poolID][block.location.poolIndex];
    }

    void VulkanMemoryAllocator::RenderMemoryUsageStat()
    {
        ImGui::Begin("Vulkan VRAM Alloc");

        for (auto& poolList : memoryPools) {
            std::string poolNodeName = std::to_string(poolList.first.blockSize) + "B Type:" + std::to_string(poolList.first.memoryTypeIndex) + " Map Type: " + std::to_string((int)poolList.first.mapUsage);
            if (ImGui::TreeNodeEx(poolNodeName.c_str(), ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_DefaultOpen)) {
                
                uint32_t totalBlocksInUse = 0;
                uint32_t totalBlocks = 0;
                for (int i = 0; i < poolList.second.size(); i++) {
                    if (sizeToBlockCountPerAlloc.find(poolList.first.blockSize) != sizeToBlockCountPerAlloc.end()) {
                        // Check how full the pool is
                        uint32_t poolMaxBlocks = sizeToBlockCountPerAlloc[poolList.first.blockSize];
                        uint32_t takenBlocks = poolMaxBlocks;
                        for (auto& location : freeMemoryLocations[poolList.first]) {
                            if (location.poolIndex == i) takenBlocks--;
                        }
                        ImGui::ProgressBar((float)takenBlocks / (float)poolMaxBlocks);
                        totalBlocksInUse += takenBlocks;
                        totalBlocks += poolMaxBlocks;
                    }
                    else {
                        ImGui::ProgressBar(1);
                        totalBlocksInUse++;
                        totalBlocks++;
                    }
                }
                if (poolList.second.size() == 0) ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "All Pools Closed");

                else { 
                    if (totalBlocks * poolList.first.blockSize < 1024) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                            std::string("Total VRAM Usage: " + std::to_string(totalBlocksInUse * poolList.first.blockSize) + "B(In Use) / " +
                                std::to_string(totalBlocks * poolList.first.blockSize) + "B(Reserved)").c_str());
                    }
                    else if (totalBlocks * poolList.first.blockSize < 1024 * 1024) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                            std::string("Total VRAM Usage: " + std::to_string(totalBlocksInUse * poolList.first.blockSize / 1024) + "KB(In Use) / " +
                                std::to_string(totalBlocks * poolList.first.blockSize / 1024) + "KB(Reserved)").c_str());
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
                            std::string("Total VRAM Usage: " + std::to_string(totalBlocksInUse * poolList.first.blockSize / 1024 / 1024) + "MB(In Use) / " +
                                std::to_string(totalBlocks * poolList.first.blockSize / 1024 / 1024) + "MB(Reserved)").c_str());
                    }
                }

                ImGui::TreePop();
            }
        }

        ImGui::End();
    }

    VulkanMemoryAllocator::VulkanMemoryBlock VulkanMemoryAllocator::FindMemoryBlock(VkDevice device, VkMemoryAllocateInfo desiredAllocation, VulkanMemoryMapUsage mapUsage)
    {
        // Find block size
        uint32_t blockSize = 0;

        for (auto& pair : sizeToBlockCountPerAlloc) {
            if (desiredAllocation.allocationSize <= pair.first) {
                blockSize = pair.first;
                break;
            }
        }

        if (blockSize != 0) {
            // Create pool identifier
            VulkanMemoryPoolIdentifier poolID = { desiredAllocation.memoryTypeIndex, blockSize, mapUsage };
            // Look to see if pool exists
            if (memoryPools.find(poolID) != memoryPools.end() && memoryPools[poolID].size() != 0) {
                // Then check if there is space
                if (freeMemoryLocations[poolID].size() != 0) {
                    // Then return the free space
                    VulkanMemoryPoolLocation location = freeMemoryLocations[poolID].front();
                    freeMemoryLocations[poolID].pop_front();
                    //std::cout << "Existing block taken, Map Usage:" << (int)poolID.mapUsage << " Memory Index:" << poolID.memoryTypeIndex << " Size: " << poolID.blockSize << "*" << sizeToBlockCountPerAlloc[poolID.blockSize] << " Index: " << location.poolIndex << " Offset: " << location.offset << "\n";
                    return { poolID, location };
                }
                //// If there isnt, make a new pool in the list (list already exists as there is at least 1 taken pool)
                //uint32_t newPoolIndex = memoryPools[poolID].size();
                //memoryPools[poolID].push_back(nullptr);
                //VkMemoryAllocateInfo allocInfo{};
                //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                //allocInfo.allocationSize = poolID.blockSize * sizeToBlockCountPerAlloc[poolID.blockSize];
                //allocInfo.memoryTypeIndex = poolID.memoryTypeIndex;
                //if (vkAllocateMemory(device, &allocInfo, nullptr, &(memoryPools[poolID][newPoolIndex])) != VK_SUCCESS) {
                //    throw - 1;
                //}
                //// Create free locations (appart from the first one, we are about to use it)
                //for (int i = 1; i < sizeToBlockCountPerAlloc[poolID.blockSize]; i++) {
                //    freeMemoryLocations[poolID].push_back({ newPoolIndex , i * blockSize});
                //}
                //// First block of the new pool is reserved
                //return { newPoolIndex, 0 };
            }
            // If the pool does not exist, create it. We dont care if its in the map, just implicitally create it if it isnt. 
            uint32_t newPoolIndex = memoryPools[poolID].size();
            memoryPools[poolID].push_back(nullptr);
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = poolID.blockSize * sizeToBlockCountPerAlloc[poolID.blockSize];
            allocInfo.memoryTypeIndex = poolID.memoryTypeIndex;
            if (vkAllocateMemory(device, &allocInfo, nullptr, &(memoryPools[poolID][newPoolIndex])) != VK_SUCCESS) {
                throw - 1;
            }
            // Open map if needed
            char* mapBase = nullptr;
            if (mapUsage == VulkanMemoryMapUsage::OPEN) {
                vkMapMemory(device, memoryPools[poolID][newPoolIndex], 0, allocInfo.allocationSize, 0, reinterpret_cast<void**>(&mapBase));
            }
            // Create free locations (appart from the first one, we are about to use it)
            for (int i = 1; i < sizeToBlockCountPerAlloc[poolID.blockSize]; i++) {
                void* blockMapLocation = (mapUsage == VulkanMemoryMapUsage::OPEN) ? mapBase + (i * blockSize) : nullptr;
                freeMemoryLocations[poolID].push_back({ newPoolIndex , i * blockSize, blockMapLocation });
            }
            //std::cout << "New pool made, Map Usage:" << (int)poolID.mapUsage << " Memory Index:" << poolID.memoryTypeIndex << " Size: " << poolID.blockSize << "*" << sizeToBlockCountPerAlloc[poolID.blockSize] << " Index: " << newPoolIndex << "\n";
            // First block of the new pool is reserved
            return { poolID, { newPoolIndex, 0, mapBase } };
        }
        // The block size we want, isnt in the list of sizes, create a 1 pool block for this as a last resort. 
        VulkanMemoryPoolIdentifier poolID = { desiredAllocation.memoryTypeIndex, desiredAllocation.allocationSize, mapUsage };
        // There is any chance a pool of this exact size also exists, so treat this as an extra pool
        uint32_t newPoolIndex = memoryPools[poolID].size();
        memoryPools[poolID].push_back(nullptr);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = poolID.blockSize;
        allocInfo.memoryTypeIndex = poolID.memoryTypeIndex;
        if (vkAllocateMemory(device, &allocInfo, nullptr, &(memoryPools[poolID][newPoolIndex])) != VK_SUCCESS) {
            throw - 1;
        }
        // Open map if needed
        void* mapBase = nullptr;
        if (mapUsage == VulkanMemoryMapUsage::OPEN) {
            vkMapMemory(device, memoryPools[poolID][newPoolIndex], 0, allocInfo.allocationSize, 0, &mapBase);
        }
        // No free locations for this, were making a 1 block pool, just its own allocation. 
        // First block of the new pool is reserved
        //std::cout << "New custom pool made, Map Usage:" << (int)poolID.mapUsage << " Memory Index:" << poolID.memoryTypeIndex << " Size: " << poolID.blockSize << " Index: " << newPoolIndex << "\n";
        return { poolID, { newPoolIndex, 0, mapBase} };
        
    }
