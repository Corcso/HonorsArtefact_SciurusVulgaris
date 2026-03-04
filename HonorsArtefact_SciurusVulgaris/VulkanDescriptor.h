#pragma once

#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include "Image.h"

	class VulkanObjectDescriptorSet
	{
	public:
		struct CombinedSamplerImageReference {
			Image* image;
			VkSampler sampler;
		};

		void Create(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

		/*void CreateAndAllocateBuffers(size_t* sizes, uint32_t bindCount, Image** images = nullptr, VkSampler* samplers = nullptr);

		void CreateDescriptorSet(VkDevice device, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool);*/

		VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }

		//void* GetMappedMemoryLocation(uint32_t bindingIndex) { return mappedMemoryLocation[bindingIndex]; }
		//
		//uint32_t GetBindingCount() { return bufferSizes.size(); }
		//size_t GetBindingSize(uint32_t bindingIndex) { return bufferSizes[bindingIndex]; }

		void UpdateUniformBufferData(uint32_t bindingIndex, void* data);
		void UpdateStorageBufferData(uint32_t bindingIndex, void* data);
		void FlushBuffer(uint32_t bindingIndex);
		void UpdateImageSampler(uint32_t bindingIndex, Image* image, VkSampler sampler);

		void CleanupDescriptor();
	private:
		struct Descriptor {
			VkDescriptorType type;
			// For UBOs
			VkBuffer buffer;
			void* openMapMemoryLocation;
			size_t bufferSize;
			VulkanMemoryAllocator::VulkanMemoryBlock bufferMemory;
			// For CISs
			VkImage* image;
			VkSampler sampler;
		};

		VkDescriptorSet descriptorSet;
		std::vector<Descriptor> descriptors;
	};
