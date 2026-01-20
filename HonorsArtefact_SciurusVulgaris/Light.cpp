#include "PCH.h"
#include "Light.h"

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
