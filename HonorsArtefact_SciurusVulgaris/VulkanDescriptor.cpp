#include "PCH.h"

#include "VulkanDescriptor.h"
#include "VulkanUtility.h"
#include "Graphics.h"

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

void VulkanObjectDescriptorSet::UpdateUniformBufferData(uint32_t bindingIndex, void* data)
{
	if (data == nullptr) return;
	if (bindingIndex >= descriptors.size()) return;
	if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return;
	// Uniform buffers stay mapped
	memcpy(descriptors[bindingIndex].openMapMemoryLocation, data, descriptors[bindingIndex].bufferSize);
}

void VulkanObjectDescriptorSet::UpdateStorageBufferData(uint32_t bindingIndex, void* data)
{
	if (data == nullptr) return;
	if (bindingIndex >= descriptors.size()) return;
	if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) return;
	// Storage buffers stay mapped
	memcpy(descriptors[bindingIndex].openMapMemoryLocation, data, descriptors[bindingIndex].bufferSize);
}

void VulkanObjectDescriptorSet::FlushBuffer(uint32_t bindingIndex)
{
	if (bindingIndex >= descriptors.size()) return;
	// Storage and Uniform Only
	if (descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER && descriptors[bindingIndex].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return;
	Graphics::GetMemoryAllocator().FlushMappedBlock(Graphics::GetVkDevice(), descriptors[bindingIndex].bufferMemory);
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
	for (int i = 0; i < descriptors.size(); i++) {
		if (descriptors[i].type != VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER && descriptors[i].type != VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) continue;
		VulkanUtility::DestroyBuffer(descriptors[i].buffer);
		VulkanUtility::FreeGPUMemoryBlock(descriptors[i].bufferMemory);
	}
}
