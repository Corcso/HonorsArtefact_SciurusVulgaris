#pragma once
#include "PCH.h"
#include "BufferStructs.h"
#include "PointTreeMesh.h"
#include "PointsToShadowMeshShade_GP.h"
#include "Light.h"

class LightShadow_RP
{
public:
	void CreateAll() {
		CreateRenderPass();

		pointsToShadowMeshShade_GP.CreateDescriptorLayout();
		pointsToShadowMeshShade_GP.CreatePipeline(vkRenderPass);
	}

	void CreateRenderPass();

	void BeginRender(Light* light, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void RenderPointTree(PointTreeMesh* points, InstancingInfo instancingInfo, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	//void SwitchToTraditionalMeshPipeline(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	//void RenderTraditionalMesh(TriListMesh* mesh, VkCommandBuffer commandBuffer = VK_NULL_HANDLE);
	void EndRender(VkCommandBuffer commandBuffer = VK_NULL_HANDLE);

	void Shutdown();

	VkRenderPass GetRenderPass() { return vkRenderPass; }
private:
	VkRenderPass vkRenderPass;

	PointsToShadowMeshShade_GP pointsToShadowMeshShade_GP;
};

