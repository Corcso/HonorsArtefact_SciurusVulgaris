#pragma once
#include "PCH.h"
#include "Image.h"

/// <summary>
/// Light class, only supports directional light. 
/// <para>Also stores shadow objects</para>
/// </summary>
class Light
{
public:
	// Buffer organised version of data stored. 
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

	// Getters and setters for each property

	void SetName(std::string name) { this->name = name; }
	void SetDirection(HMM_Vec3 direction) { this->direction = direction; }
	void SetColor(HMM_Vec3 color) { this->color = color; }
	void SetAmbientColor(HMM_Vec3 color) { this->ambientColor = color; }
	void SetIntensity(float intensity) { this->intensity = intensity; }
	void SetAmbientIntensity(float intensity) { this->ambientIntensity = intensity; }

	std::string GetName() {return name; }
	HMM_Vec3 GetDirection() {return direction; }
	HMM_Vec3 GetColor() {return color; }
	HMM_Vec3 GetAmbientColor() {return ambientColor; }
	float GetIntensity() { return intensity; }
	float GetAmbientIntensity() { return ambientIntensity; }

	/// <summary>
	/// Get orthographic projection matrix
	/// </summary>
	/// <param name="radius">World size of U and V in the output</param>
	/// <param name="backFactor">How far back to start rendering</param>
	/// <param name="forwardsFactor">How far forward to end rendering</param>
	/// <returns>Matrix</returns>
	HMM_Mat4 GetProjectionMatrix(float radius, float backFactor, float forwardsFactor);

	/// <summary>
	/// Get view matrix
	/// </summary>
	/// <param name="focusPoint">The centerpoint to render from.</param>
	/// <returns>Matrix</returns>
	HMM_Mat4 GetViewMatrix(HMM_Vec3 focusPoint);

	/// <summary>
	/// Get buffer formatted data for this light
	/// </summary>
	BufferStruct GetBufferData();

	/// <summary>
	/// Render imgui controls. 
	/// </summary>
	/// <param name="createWindow">If a new window should be created in imgui or not</param>
	void RenderImGuiMenu(bool createWindow = false);

	/// <summary>
	/// Create shadow resources. Images, Imgui Image reference, and frame buffer
	/// </summary>
	void CreateShadowResources(VkRenderPass vkRenderPass);
	/// <summary>
	/// Destroy shadow resources.
	/// </summary>
	void ShutdownShadowResources();

	Image* GetShadowImage() { return &shadowImage; }
	ImTextureID GetShadowImageImGuiTex() { return shadowImageImGuiTex; }
	VkFramebuffer GetShadowFrameBuffer() { return shadowFrameBuffer; }

	void SetShadowEnabled(bool enabled) { this->shadowEnabled = enabled; }
	bool IsShadowEnabled() { return shadowEnabled; }
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

