#include "PCH.h"
#include "Light.h"
#include "Graphics.h"
#include "VulkanSetup.h"

HMM_Mat4 Light::GetProjectionMatrix(float radius, float backFactor, float forwardsFactor)
{
    projMatrix = HMM_Orthographic_RH_ZO(-radius, radius, -radius, radius, -backFactor, forwardsFactor);
    return projMatrix;
}

HMM_Mat4 Light::GetViewMatrix(HMM_Vec3 focusPoint)
{
    HMM_Vec3 nonParallelVector = HMM_V3(0, 1, 0);
    if (HMM_Dot(nonParallelVector, direction) < 0.01) nonParallelVector = HMM_V3(0, 0, 1);
    viewMatrix = HMM_LookAt_LH(focusPoint, focusPoint - direction, HMM_Cross(nonParallelVector, -direction));
    return viewMatrix;
}

Light::BufferStruct Light::GetBufferData()
{
	return {
		direction, 0, color, intensity, viewMatrix, projMatrix
	};
}

void Light::RenderImGuiMenu(bool createWindow)
{
	if (createWindow) ImGui::Begin(("Light: " + name).c_str());
	else ImGui::SeparatorText(("Light: " + name).c_str());
	ImGui::DragFloat3("Direction", reinterpret_cast<float*>(&direction), 0.05, -1.0, 1.0);
	ImGui::ColorPicker3("Color", reinterpret_cast<float*>(&color));
	ImGui::DragFloat("Intensity", &intensity, 0.05, 0.0, 100.0);

	if (createWindow) ImGui::End();
}

void Light::CreateShadowResources(VkRenderPass vkRenderPass)
{
    shadowImage.CreateImage(VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()), 2048, 2048, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
    shadowImage.CreateImageView(true);

    shadowImageImGuiTex = reinterpret_cast<ImTextureID>(
        ImGui_ImplVulkan_AddTexture(Graphics::GetBasicLinearSampler(), shadowImage.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    );
   
    VkImageView imageViewList[]{ shadowImage.GetImageView() };

    VkFramebufferCreateInfo frameBufferCreateInfo{};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.width = shadowImage.GetImageExtent().width;
    frameBufferCreateInfo.height = shadowImage.GetImageExtent().height;
    frameBufferCreateInfo.attachmentCount = 1;
    frameBufferCreateInfo.pAttachments = imageViewList;
    frameBufferCreateInfo.renderPass = vkRenderPass;
    frameBufferCreateInfo.layers = 1;

    if (vkCreateFramebuffer(Graphics::GetVkDevice(), &frameBufferCreateInfo, nullptr, &shadowFrameBuffer) != VK_SUCCESS) {
        throw - 1;
    }
}

void Light::ShutdownShadowResources()
{
    vkDestroyFramebuffer(Graphics::GetVkDevice(), shadowFrameBuffer, nullptr);
    shadowImage.Destroy();
}
