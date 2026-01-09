#include "PCH.h"

#include "VulkanDescriptor.h"
#include "VulkanUtility.h"
#include "Graphics.h"

	/*void VulkanDescriptor::CreateAndAllocateBuffer(size_t size)
	{
		VulkanUtility::CreateBufferAndAssignMemory(size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&descriptorBuffer, &descriptorBufferMemory);

		mappedMemoryLocation = VulkanUtility::OpenMemoryMap(descriptorBufferMemory, size);

		bufferSize = size;
	}*/

void VulkanObjectDescriptorSet::Create(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes)
{
	descriptors.resize(layoutInformation.bindingCount);

	// Get descriptor binding information
	for (uint32_t descriptorIndex = 0; descriptorIndex < layoutInformation.bindingCount; descriptorIndex++) {
		VkDescriptorSetLayoutBinding binding = layoutInformation.pBindings[descriptorIndex];

		descriptors[descriptorIndex].type = binding.descriptorType;
		if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			descriptors[descriptorIndex].bufferSize = sizes[descriptorIndex];

			VulkanUtility::CreateBufferAndAssignMemory(sizes[descriptorIndex], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&(descriptors[descriptorIndex].buffer), &(descriptors[descriptorIndex].bufferMemory), VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN);

			descriptors[descriptorIndex].openMapMemoryLocation = descriptors[descriptorIndex].bufferMemory.location.openMap;
		}
		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
			descriptors[descriptorIndex].bufferSize = sizes[descriptorIndex];

			VulkanUtility::CreateBufferAndAssignMemory(sizes[descriptorIndex], VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				&(descriptors[descriptorIndex].buffer), &(descriptors[descriptorIndex].bufferMemory), VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN);

			descriptors[descriptorIndex].openMapMemoryLocation = descriptors[descriptorIndex].bufferMemory.location.openMap;
		}
		else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
			descriptors[descriptorIndex].image = nullptr;
			descriptors[descriptorIndex].sampler = VK_NULL_HANDLE;
		}
	}

	// Create descriptor set
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = Graphics::GetDescriptorPool();
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	if (vkAllocateDescriptorSets(Graphics::GetVkDevice() , &allocInfo, &descriptorSet) != VK_SUCCESS) {
		throw - 1;
	}

	// Update all UBOS
	// Could this be one API call?
	for (uint32_t descriptorIndex = 0; descriptorIndex < layoutInformation.bindingCount; descriptorIndex++) {
		if (descriptors[descriptorIndex].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = descriptors[descriptorIndex].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = descriptors[descriptorIndex].bufferSize;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = descriptorIndex;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(Graphics::GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
		}
		else if (descriptors[descriptorIndex].type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = descriptors[descriptorIndex].buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = descriptors[descriptorIndex].bufferSize;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = descriptorIndex;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptorWrite.descriptorCount = 1;

			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(Graphics::GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}
}

//void VulkanObjectDescriptorSet::CreateAndAllocateBuffers(size_t* sizes, uint32_t bindCount, Image** images, VkSampler* samplers)
//	{
//		// Image images is set, sampler must be too
//		if (images == nullptr && samplers != nullptr) {
//			throw - 1;
//		}
//		else if (samplers == nullptr && images != nullptr) {
//			throw - 1;
//		}
//
//		bindingCount = bindCount;
//		descriptorBuffer.resize(bindCount);
//		descriptorBufferMemory.resize(bindCount);
//		mappedMemoryLocation.resize(bindCount);
//		bufferSizes.resize(bindCount);
//		for (int i = 0; i < bindCount; i++) {
//			// Don't create a buffer, if this is an image descriptor
//			if (images != nullptr && images[i] != nullptr) continue;
//
//			VulkanUtility::CreateBufferAndAssignMemory(sizes[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//				&(descriptorBuffer[i]), &(descriptorBufferMemory[i]), VulkanMemoryAllocator::VulkanMemoryMapUsage::OPEN);
//
//			mappedMemoryLocation[i] = descriptorBufferMemory[i].location.openMap; //VulkanUtility::OpenMemoryBlockMap(descriptorBufferMemory[i], sizes[i]);
//
//			bufferSizes[i] = sizes[i];
//		}
//		// Copy image binding array to vector
//		this->images.resize(bindCount, nullptr);
//		this->samplers.resize(bindCount, VK_NULL_HANDLE);
//		if (images != nullptr) {
//			for (int i = 0; i < bindCount; i++) {
//				this->images[i] = images[i];
//				this->samplers[i] = samplers[i];
//			}
//		}
//	}
//
//	void VulkanObjectDescriptorSet::CreateDescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool)
//	{
//		VkDescriptorSetAllocateInfo allocInfo{};
//		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//		allocInfo.descriptorPool = descriptorPool;
//		allocInfo.descriptorSetCount = 1;
//		allocInfo.pSetLayouts = &layout;
//
//		if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
//			throw -1;
//		}
//
//		for (int i = 0; i < bindingCount; i++) {
//			VkDescriptorBufferInfo bufferInfo{};
//			bufferInfo.buffer = descriptorBuffer[i];
//			bufferInfo.offset = 0;
//			bufferInfo.range = bufferSizes[i];
//
//			VkDescriptorImageInfo imageInfo{};
//			if (images[i] != nullptr) {
//				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//				imageInfo.imageView = images[i]->GetImageView();
//				imageInfo.sampler = samplers[i];
//			}
//			VkWriteDescriptorSet descriptorWrite{};
//			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//			descriptorWrite.dstSet = descriptorSet;
//			descriptorWrite.dstBinding = i;
//			descriptorWrite.dstArrayElement = 0;
//			descriptorWrite.descriptorType = (images[i] != nullptr) ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//			descriptorWrite.descriptorCount = 1;
//			if (images[i] != nullptr) {
//				descriptorWrite.pBufferInfo = nullptr;
//				descriptorWrite.pImageInfo = &imageInfo; 
//			}
//			else {
//				descriptorWrite.pBufferInfo = &bufferInfo;
//				descriptorWrite.pImageInfo = nullptr;
//			}
//			descriptorWrite.pTexelBufferView = nullptr; // Optional
//			vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
//		}
//	}

	void VulkanObjectDescriptorSet::UpdateUniformBufferData(uint32_t bindingIndex, void* data)
	{
		if (data == nullptr) return;
		if (bindingIndex >= descriptors.size()) return;
		if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return;
		memcpy(descriptors[bindingIndex].openMapMemoryLocation, data, descriptors[bindingIndex].bufferSize);
	}

	void VulkanObjectDescriptorSet::UpdateStorageBufferData(uint32_t bindingIndex, void* data)
	{
		if (data == nullptr) return;
		if (bindingIndex >= descriptors.size()) return;
		if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) return;
		memcpy(descriptors[bindingIndex].openMapMemoryLocation, data, descriptors[bindingIndex].bufferSize);
	}

	void VulkanObjectDescriptorSet::UpdateImageSampler(uint32_t bindingIndex, Image* image, VkSampler sampler)
	{
		if (bindingIndex >= descriptors.size()) return;
		if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) return;
		if (image == nullptr) return;
		if (sampler == VK_NULL_HANDLE) return;

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = image->GetImageView();
		imageInfo.sampler = sampler;
		
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = bindingIndex;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imageInfo;
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		vkUpdateDescriptorSets(Graphics::GetVkDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanObjectDescriptorSet::CleanupDescriptor()
	{
		// Close map 
		// TODO
		// and free memory
		for (int i = 0; i < descriptors.size(); i++) {
			if (descriptors[i].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptors[i].type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) continue;
			VulkanUtility::DestroyBuffer(descriptors[i].buffer);
			VulkanUtility::FreeGPUMemoryBlock(descriptors[i].bufferMemory);
		}
	}
