#pragma once
#include "PCH.h"

class Light
{
public:
	struct BufferStruct {
		HMM_Vec3 direction; float p_0;
		HMM_Vec3 color; 
		float intensity;
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
	}

	void SetName(std::string name) { this->name = name; }
	void SetDirection(HMM_Vec3 direction) { this->direction = direction; }
	void SetColor(HMM_Vec3 color) { this->color = color; }
	void SetIntensity(float intensity) { this->intensity = intensity; }

	std::string GetName() {return name; }
	HMM_Vec3 GetDirection() {return direction; }
	HMM_Vec3 GetColor() {return color; }
	float GetIntensity() { return intensity; }

	BufferStruct GetBufferData();

	void RenderImGuiMenu(bool createWindow = false);

private:
	Type myType;

	std::string name;

	HMM_Vec3 direction;
	HMM_Vec3 color;
	float intensity;
};

