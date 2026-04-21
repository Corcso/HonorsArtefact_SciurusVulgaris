#include "PCH.h"
#include "Transform.h"
#include "Input.h"

Transform::Transform()
{
	position = HMM_V3(0, 0, 0);
	euler = HMM_V3(0, 0, 0);
	scale = HMM_V3(1, 1, 1);
}

void Transform::UpdateMatrix()
{
	// Calculate Matrix
	HMM_Mat4 combinedRotation = HMM_Rotate_LH(euler.Y * HMM_DegToRad, HMM_V3(0, 1, 0)) * HMM_Rotate_LH(euler.X * HMM_DegToRad, HMM_V3(1, 0, 0)) * HMM_Rotate_LH(euler.Z * HMM_DegToRad, HMM_V3(0, 0, 1));
	matrix = HMM_Translate(position) * combinedRotation * HMM_Scale(scale);

	// Calc Directions 
	forward = HMM_MulM4V4(combinedRotation, HMM_V4(0, 0, 1, 0)).XYZ;
	up = HMM_MulM4V4(combinedRotation, HMM_V4(0, 1, 0, 0)).XYZ;
	right = HMM_MulM4V4(combinedRotation, HMM_V4(1, 0, 0, 0)).XYZ;
}

void Transform::DisplayController(uint32_t id)
{
	// Display UI
	ImGui::PushID(id);
	ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position), 0.1f);
	ImGui::DragFloat3("Angles", reinterpret_cast<float*>(&euler), 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scale), 0.1f);
	ImGui::PopID();

	UpdateMatrix();
}

CameraTransform::CameraTransform()
{
	speed = 0.01f;
}

void CameraTransform::UpdateMatrix()
{
	Transform::UpdateMatrix();

	viewMatrix = HMM_LookAt_LH(position, position - forward, -up);
}

const float CameraTransform::ANGLE_SPEED_MULTIPLIER = 3;

void CameraTransform::CaptureControls()
{
	// Translation
	if (Input::IsKeyDown('W')) {
		position += forward * speed;
	}
	else if (Input::IsKeyDown('S')) {
		position -= forward * speed;
	}
	if (Input::IsKeyDown('E')) {
		position += up * speed;
	}
	else if (Input::IsKeyDown('Q')) {
		position -= up * speed;
	}
	if (Input::IsKeyDown('D')) {
		position += right * speed;
	}
	else if (Input::IsKeyDown('A')) {
		position -= right * speed;
	}

	// Rotation
	if (Input::IsKeyPressed('P')) {
		mouseLocked = !mouseLocked;
		Input::SetMouseLock(mouseLocked);
	}
	if (mouseLocked) {
		euler.X -= speed * Input::GetMousePositionDifference().Y;
		euler.Y -= speed * Input::GetMousePositionDifference().X;
	}

	// Speed
	if (Input::IsKeyPressed('X')) {
		speed += 0.05f;
	}
	else if (Input::IsKeyPressed('Z')) {
		speed -= 0.05f;
	}

	UpdateMatrix();
}
