#include "PCH.h"
#include "Transform.h"

void Transform::DisplayController(uint32_t id)
{
	// Display UI
	ImGui::PushID(id);
	ImGui::DragFloat3("Position", reinterpret_cast<float*>(&position), 0.1f);
	ImGui::DragFloat3("Angles", reinterpret_cast<float*>(&euler), 1.0f, -180.0f, 180.0f);
	ImGui::DragFloat3("Scale", reinterpret_cast<float*>(&scale), 0.1f);
	ImGui::PopID();

	// Calculate Matrix
	// TODO Figure out the right order here.
	matrix = HMM_Translate(position) * (HMM_Rotate_LH(euler.Z * HMM_DegToRad, HMM_V3(0, 0, 1)) * HMM_Rotate_LH(euler.Y * HMM_DegToRad, HMM_V3(1, 0, 0)) * HMM_Rotate_LH(euler.X * HMM_DegToRad, HMM_V3(0, 0, 1))) * HMM_Scale(scale);
}
