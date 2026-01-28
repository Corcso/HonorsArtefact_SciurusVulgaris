#include "PCH.h"
#include "Light.h"
#include "Graphics.h"
#include "VulkanSetup.h"

Light::BufferStruct Light::GetBufferData()
{
	return {
		direction, 0, color, intensity
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
    shadowImage.CreateImage(VulkanSetup::GetDepthBufferFormat(Graphics::GetVkPhysicalDevice()), Graphics::GetSwapChainExtent().width, Graphics::GetSwapChainExtent().height, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    shadowImage.CreateImageView(true);
   
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
