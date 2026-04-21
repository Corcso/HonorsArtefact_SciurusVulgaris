#pragma once

#include "PCH.h"
#include "VulkanMemoryAllocator.h"
#include "Image.h"

/// <summary>
/// Object Descriptor Set, for use with rendered objects to store all descriptors. 
/// Stores image & sampler references. But stores buffers internally.
/// </summary>
class VulkanObjectDescriptorSet
{
public:
	/// <summary>
	/// Struct for a combined image sampler reference. 
	/// </summary>
	struct CombinedSamplerImageReference {
		Image* image;
		VkSampler sampler;
	};

	/// <summary>
	/// Create this descriptor set based on the layout, layout creation info and size of buffer list provided.
	/// Image Sampler sizes are ignored. 
	/// </summary>
	/// <param name="layout">Descriptor Set Layout</param>
	/// <param name="layoutInformation">Desciptor Set Layout Creation Info</param>
	/// <param name="sizes">List of buffer sizes</param>
	void Create(VkDescriptorSetLayout layout, VkDescriptorSetLayoutCreateInfo layoutInformation, size_t* sizes);

	/// <summary>
	/// Get the Descriptor set vulkan object
	/// </summary>
	VkDescriptorSet* GetDescriptorSet() { return &descriptorSet; }

	/// <summary>
	/// Update uniform buffer data. Copies the entire size of the buffer. 
	/// </summary>
	/// <param name="bindingIndex">Index of buffer</param>
	/// <param name="data">Data location</param>
	void UpdateUniformBufferData(uint32_t bindingIndex, void* data);
	/// <summary>
	/// Update storage buffer data. Copies the entire size of the buffer. 
	/// </summary>
	/// <param name="bindingIndex">Index of buffer</param>
	/// <param name="data">Data location</param>
	void UpdateStorageBufferData(uint32_t bindingIndex, void* data);
	/// <summary>
	/// Call flush memory ranges on the buffer
	/// </summary>
	/// <param name="bindingIndex">Index of buffer</param>
	void FlushBuffer(uint32_t bindingIndex);

	/// <summary>
	/// Update an image sampler reference. 
	/// </summary>
	/// <param name="bindingIndex">Index of Combined Image Sampler</param>
	/// <param name="image">Image</param>
	/// <param name="sampler">Sampler</param>
	void UpdateImageSampler(uint32_t bindingIndex, Image* image, VkSampler sampler);

	void CleanupDescriptor();
private:
	/// <summary>
	/// A single 1 binding descriptor struct. 
	/// UBO section is used for uniform and storage buffers, CIS for combined image samplers. 
	/// </summary>
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
	// Descriptor set and list of descriptors. 
	VkDescriptorSet descriptorSet;
	std::vector<Descriptor> descriptors;
};
