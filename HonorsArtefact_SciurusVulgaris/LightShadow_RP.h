#pragma once
#include "PCH.h"
#include "BufferStructs.h"
#include "PointTreeMesh.h"
#include "PointsToShadowMeshShade_GP.h"
#include "Light.h"

/// <summary>
/// Render pass for rendering points to the depth buffer for shadowing. 
/// </summary>
class LightShadow_RP
{
public:
	/// <summary>
	/// Create all resources. 
	/// </summary>
	void CreateAll() {
		CreateRenderPass();

		pointsToShadowMeshShade_GP.CreateDescriptorLayout();
		pointsToShadowMeshShade_GP.CreatePipeline(vkRenderPass);
	}

	void CreateRenderPass();

	/// <summary>
	/// Begin the render pass for the provided light
	/// </summary>
	/// <param name="light">Light's shadow map to render</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void BeginRender(Light* light, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// Render a point tree to the shadow map
	/// </summary>
	/// <param name="points">Point Tree</param>
	/// <param name="instancingInfo">Instancing info</param>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void RenderPointTree(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	/// <summary>
	/// End shadow map render for this light
	/// </summary>
	/// <param name="commandBuffer">Command buffer to use, will use frame's if none provided. </param>
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	VkDescriptorSetLayout GetShadowSetLayout() { return pointsToShadowMeshShade_GP.vkDescriptorSetLayout; }
	VkDescriptorSetLayoutCreateInfo GetShadowSetLayoutInfo() { return pointsToShadowMeshShade_GP.vkDescriptorSetLayoutInfo; }

	void Shutdown();

	VkRenderPass GetRenderPass() { return vkRenderPass; }
private:
	VkRenderPass vkRenderPass;

	PointsToShadowMeshShade_GP pointsToShadowMeshShade_GP;
};

