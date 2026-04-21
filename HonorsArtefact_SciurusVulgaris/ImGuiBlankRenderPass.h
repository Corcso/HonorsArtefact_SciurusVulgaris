#pragma once
#include "VulkanMemoryAllocator.h"
#include "VulkanDescriptor.h"

/// <summary>
/// Blank Pass for ImGui UI only. 
/// <para>An old unified renderer OUR/UGP, Render passes and graphics pipelines combined into 1 class</para>
/// </summary>
class ImGuiBlankRenderPass
{
public:
	void CreateAll() {
	}
	void Shutdown();

	/// <summary>
	/// Begin the render pass
	/// </summary>
	/// <param name="clearColor">Clear colour</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRender(HMM_Vec4 clearColor, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// End the render pass
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetDescriptorSetLayout() { return vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetDescriptorSetLayoutInfo() { return vkDescriptorSetLayoutInfo; }

private:
	VkRenderPass vkRenderPass;
	VkDescriptorSetLayout vkDescriptorSetLayout;
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutInfo;
};

