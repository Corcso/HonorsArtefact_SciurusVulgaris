#pragma once
#include "PCH.h"
#include "Image.h"

class Light
{
public:
	struct BufferStruct {
		HMM_Vec3 direction; float p_0;
		HMM_Vec3 color; 
		float intensity;
		HMM_Mat4 viewMatrix;
		HMM_Mat4 projMatrix;
		HMM_Vec3 ambientColor;
		float ambientIntensity;
		uint32_t shadowEnabled;
	};

	enum class Type {
		DIRECTIONAL
	};

	Light() {
		name = "Light";
		myType = Type::DIRECTIONAL;
		direction = HMM_V3(-0.707, -0.707, 0);
		color = HMM_V3(1, 1, 1);
		intensity = 1;
		ambientColor = HMM_V3(1, 1, 1);
		ambientIntensity = 0.1;
		shadowEnabled = false;
	}

	void SetName(std::string name) { this->name = name; }
	void SetDirection(HMM_Vec3 direction) { this->direction = direction; }
	void SetColor(HMM_Vec3 color) { this->color = color; }
	void SetIntensity(float intensity) { this->intensity = intensity; }

	std::string GetName() {return name; }
	HMM_Vec3 GetDirection() {return direction; }
	HMM_Vec3 GetColor() {return color; }
	float GetIntensity() { return intensity; }

	HMM_Mat4 GetProjectionMatrix(float radius, float backFactor, float forwardsFactor);
	HMM_Mat4 GetViewMatrix(HMM_Vec3 focusPoint);

	BufferStruct GetBufferData();

	void RenderImGuiMenu(bool createWindow = false);

	void CreateShadowResources(VkRenderPass vkRenderPass);
	void ShutdownShadowResources();

	Image* GetShadowImage() { return &shadowImage; }
	ImTextureID GetShadowImageImGuiTex() { return shadowImageImGuiTex; }
	VkFramebuffer GetShadowFrameBuffer() { return shadowFrameBuffer; }

private:
	Type myType;

	std::string name;

	HMM_Vec3 direction;
	HMM_Vec3 color;
	float intensity;
	HMM_Vec3 ambientColor;
	float ambientIntensity;

	Image shadowImage;
	ImTextureID shadowImageImGuiTex;
	VkFramebuffer shadowFrameBuffer;

	HMM_Mat4 viewMatrix;
	HMM_Mat4 projMatrix;

	bool shadowEnabled;
};

