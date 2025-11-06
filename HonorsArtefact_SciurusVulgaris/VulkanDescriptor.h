#pragma once

#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include "Image.h"

	class VulkanDescriptor
	{
	public:
		void CreateAndAllocateBuffers(size_t* sizes, uint32_t bindCount, Image** images = nullptr, VkSampler* samplers = nullptr);

		void CreateDescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool);

		VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }

		void* GetMappedMemoryLocation(uint32_t bindingIndex) { return mappedMemoryLocation[bindingIndex]; }
		
		void CleanupDescriptor();
	private:
		VkDescriptorSet descriptorSet;
		std::vector<VkBuffer> descriptorBuffer;
		//std::vector<VkDeviceMemory> descriptorBufferMemory;
		std::vector<VulkanMemoryAllocator::VulkanMemoryBlock> descriptorBufferMemory;
		std::vector<void*> mappedMemoryLocation;
		std::vector<size_t> bufferSizes;
		std::vector<Image*> images;
		std::vector<VkSampler> samplers;
		uint32_t bindingCount;
	};
