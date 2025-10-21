#include "PCH.h"

#include "VulkanDescriptor.h"
#include "VulkanUtility.h"

	/*void VulkanDescriptor::CreateAndAllocateBuffer(size_t size)
	{
		VulkanUtility::CreateBufferAndAssignMemory(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&descriptorBuffer, &descriptorBufferMemory);

		mappedMemoryLocation = VulkanUtility::OpenMemoryMap(descriptorBufferMemory, size);

		bufferSize = size;
	}*/

	void VulkanDescriptor::CreateAndAllocateBuffers(size_t* sizes, uint32_t bindCount)
	{
		bindingCount = bindCount;
		descriptorBuffer.resize(bindCount);
		descriptorBufferMemory.resize(bindCount);
		mappedMemoryLocation.resize(bindCount);
		bufferSizes.resize(bindCount);
		for (int i = 0; i < bindCount; i++) {
			VulkanUtility::CreateBufferAndAssignMemory(sizes[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&(descriptorBuffer[i]), &(descriptorBufferMemory[i]), VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN);

			mappedMemoryLocation[i] = descriptorBufferMemory[i].location.openMap; //VulkanUtility::OpenMemoryBlockMap(descriptorBufferMemory[i], sizes[i]);

			bufferSizes[i] = sizes[i];
		}
	}

	void VulkanDescriptor::CreateDescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw -1;
		}

		for (int i = 0; i < bindingCount; i++) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = descriptorBuffer[i];
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSizes[i];

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = i;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional
			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDescriptor::CleanupDescriptor()
	{
		// Close map 
		// TODO
		// and free memory
		for (int i = 0; i < bindingCount; i++) {
			VulkanUtility::DestroyBuffer(descriptorBuffer[i]);
			//VulkanUtility::FreeGPUMemory(descriptorBufferMemory[i]);
			VulkanUtility::FreeGPUMemoryBlock(descriptorBufferMemory[i]);
		}
	}
