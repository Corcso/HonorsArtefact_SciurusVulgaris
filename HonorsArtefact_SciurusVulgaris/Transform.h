#pragma once
#include "PCH.h"

/// <summary>
/// Transform class which stores position, rotation and scale
/// </summary>
class Transform
{
	// Fully public for ease of development and access. No variables are sensitive.
public:
	Transform();

	HMM_Mat4 matrix;

	/// <summary>
	/// Updates the transformation matrix. Must be called before using matrix.
	/// <para>Also calculates up, forward and right</para>
	/// </summary>
	virtual void UpdateMatrix();

	/// <summary>
	/// Display ImGui Position, rotation and scale editor. 
	/// </summary>
	/// <param name="id">ImGui ID to push if needed.</param>
	void DisplayController(uint32_t id = 0);

	HMM_Vec3 position;
	HMM_Vec3 euler;
	HMM_Vec3 scale;

	HMM_Vec3 forward;
	HMM_Vec3 up;
	HMM_Vec3 right;
};

/// <summary>
/// A camera version of transform which takes camera movement input. 
/// </summary>
class CameraTransform : public Transform {
public:
	CameraTransform();

	HMM_Mat4 viewMatrix;
	float speed;

	void UpdateMatrix() override;

	/// <summary>
	/// Should be ran at the start of each frame to update the transformations with input. 
	/// </summary>
	void CaptureControls();
	bool mouseLocked;
private:
	const static float ANGLE_SPEED_MULTIPLIER;
};