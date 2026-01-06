#pragma once
#include "PCH.h"

class Transform
{
public:
	Transform();

	HMM_Mat4 matrix;

	virtual void UpdateMatrix();
	void DisplayController(uint32_t id = 0);

	HMM_Vec3 position;
	HMM_Vec3 euler;
	HMM_Vec3 scale;

	HMM_Vec3 forward;
	HMM_Vec3 up;
	HMM_Vec3 right;
};

class CameraTransform : public Transform {
public:
	CameraTransform();

	HMM_Mat4 viewMatrix;
	float speed;

	void UpdateMatrix() override;
	void CaptureControls();
private:
	const static float ANGLE_SPEED_MULTIPLIER;
};