#include "PCH.h"
#include "ImGuiHelpers.h"

ImGuiHelpers::MultiTriListMeshLoader::MultiTriListMeshLoader()
{
	meshPath.resize(256);
	texturesEnabled.resize(8);
	texturePaths.resize(8);
	for (int i = 0; i < 8; i++) {
		texturePaths[i].resize(256);
	}
	// Setup Presets
	SetupPresetDictionary();
	// Set First Preset to Loaded
	meshPath = presetDictionary[AVAILABLE_PRESETS[0]].meshPath;
	texturePaths = presetDictionary[AVAILABLE_PRESETS[0]].texturePaths;
	texturesEnabled = presetDictionary[AVAILABLE_PRESETS[0]].texturesEnabled;
}

void ImGuiHelpers::MultiTriListMeshLoader::Display(std::function<void(TriListMesh*, Image*)> setupDescriptors)
{
	ImGui::Text("Currently Loaded: ");
	ImGui::SameLine();
	ImGui::Text(meshPathCurrentlyLoaded == "" ? "None" : meshPathCurrentlyLoaded.c_str());
	int lastChosenIndex = currentlyChosenPresetIndex;
	ImGui::Combo("Preset Options", &currentlyChosenPresetIndex, AVAILABLE_PRESETS, (int)AVAILABLE_PRESET_COUNT);

	if (lastChosenIndex != currentlyChosenPresetIndex) {
		meshPath = presetDictionary[AVAILABLE_PRESETS[currentlyChosenPresetIndex]].meshPath;
		texturePaths = presetDictionary[AVAILABLE_PRESETS[currentlyChosenPresetIndex]].texturePaths;
		texturesEnabled = presetDictionary[AVAILABLE_PRESETS[currentlyChosenPresetIndex]].texturesEnabled;
	}

	ImGui::InputText("Model Path", meshPath.data(), 256);
	// Texture selection & enabled menu
	ImGui::Text("Textures");
	for (int i = 0; i < 8; i++) {
		ImGui::PushID(i);
		ImGui::PushID(1);
		ImGui::Checkbox("", reinterpret_cast<bool*>(&texturesEnabled[i]));
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::PushID(2);
		ImGui::InputText("", texturePaths[i].data(), 256);
		ImGui::PopID();
		ImGui::PopID();
	}
	if (ImGui::Button("Load")) {
		// Load mesh and texture
		mesh.clear();
		textures.clear();
		textures.resize(8);

		for (int i = 0; i < 8; i++) {
			if (texturesEnabled[i]) {
				textures[i].CreateAndLoadImageFromFile(texturePaths[i], VK_IMAGE_USAGE_SAMPLED_BIT);
				textures[i].CreateImageView();
			}
		}

		mesh = TriListMesh::LoadMultiMeshFile(meshPath);
		// Now call setupDescriptors lambda for each mesh and texture pair
		for (int i = 0; i < mesh.size(); i++) {
			setupDescriptors(&mesh[i], texturesEnabled[i] ? &textures[i] : nullptr);
		}

		meshPathCurrentlyLoaded = meshPath;
	}
}

void ImGuiHelpers::MultiTriListMeshLoader::Cleanup()
{
	mesh.clear();
	textures.clear();
}

std::string ImGuiHelpers::MultiTriListMeshLoader::WhatIsLoaded()
{
	return meshPathCurrentlyLoaded;
}

void ImGuiHelpers::MultiTriListMeshLoader::SwapToPreset(std::string presetName)
{
	meshPath = presetDictionary[presetName].meshPath;
	texturePaths = presetDictionary[presetName].texturePaths;
	texturesEnabled = presetDictionary[presetName].texturesEnabled;
}

void ImGuiHelpers::MultiTriListMeshLoader::LoadNow(std::function<void(TriListMesh*, Image*)> setupDescriptors)
{
	mesh.clear();
	textures.clear();
	textures.resize(8);

	for (int i = 0; i < 8; i++) {
		if (texturesEnabled[i]) {
			textures[i].CreateAndLoadImageFromFile(texturePaths[i], VK_IMAGE_USAGE_SAMPLED_BIT);
			textures[i].CreateImageView();
		}
	}

	mesh = TriListMesh::LoadMultiMeshFile(meshPath);
	for (int i = 0; i < mesh.size(); i++) {
		setupDescriptors(&mesh[i], texturesEnabled[i] ? &textures[i] : nullptr);
	}

	meshPathCurrentlyLoaded = meshPath;
}

void ImGuiHelpers::MultiTriListMeshLoader::SetupPresetDictionary()
{
	// (Unity Technologies, 2025)
	presetDictionary[AVAILABLE_PRESETS[0] /*Summer Bubble*/] = {
		"./models/SpeedTrees/SpeedTree.obj",
		{ "./models/SpeedTrees/singleAColor.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", },
		{ true, true, false, false, false, false, false, false }
	};
	// (Unity Technologies, 2025)
	presetDictionary[AVAILABLE_PRESETS[1] /*Autumn Bubble*/] = {
		"./models/SpeedTrees/SpeedTree.obj",
		{ "./models/SpeedTrees/singleAColor_autumn.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", },
		{ true, true, false, false, false, false, false, false }
	};
	presetDictionary[AVAILABLE_PRESETS[2] /*512 Triangle Plane*/] = {
		"./models/Testing/512TrianglePlane.obj",
		{ "./models/SpeedTrees/singleAColor.png", "", "", "",  "",  "",  "",  "", },
		{ true, false, false, false, false, false, false, false }
	};
	// (Unity Technologies, 2025)
	presetDictionary[AVAILABLE_PRESETS[3] /*Pine 001*/] = {
		"./models/SpeedTrees/Pine001/Model.fbx",
		{ "./models/SpeedTrees/Pine001/Pine_Bark.png", "./models/SpeedTrees/Pine001/Pine_Bark.png", "./models/SpeedTrees/Pine001/Material_Leaf_Example_Combined.png", "",  "",  "",  "",  "", },
		{ true, true, true, false, false, false, false, false }
	};
	// (Unity Technologies, 2025)
	presetDictionary[AVAILABLE_PRESETS[4] /*Conifer 001*/] = {
		"./models/SpeedTrees/Conifer001/Model.fbx",
		{"./models/SpeedTrees/Conifer001/Example_Combined.png", "./models/SpeedTrees/Pine001/Pine_Bark.png",  "", "",  "",  "",  "",  "",},
		{ true, true, false, false, false, false, false, false }
	};
}

void ImGuiHelpers::ImGuiSetupStyle()
{
	// Style from (Thomet, 2026)
	// https://pthom.github.io/imgui_explorer/
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;

	style.WindowPadding = ImVec2(8, 8);
	style.FramePadding = ImVec2(4, 3);
	style.ItemSpacing = ImVec2(8, 4);
	style.ItemInnerSpacing = ImVec2(4, 4);
	style.TouchExtraPadding = ImVec2(0, 0);

	style.IndentSpacing = 21;
	style.GrabMinSize = 12;

	style.ChildRounding = 2;
	style.FrameRounding = 2;
	style.GrabRounding = 2;
	style.PopupRounding = 2;
	style.WindowRounding = 2;

	style.ScrollbarRounding = 12;
	style.ScrollbarSize = 12;

	style.TabBarBorderSize = 1;
	style.TabBorderSize = 0;
	style.TabBarOverlineSize = 1;
	style.TabRounding = 3;

	ImVec4* colors = style.Colors;
	colors[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.15f, 0.86f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.15f, 0.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.14f, 0.15f, 0.86f);
	colors[ImGuiCol_Border] = ImVec4(0.52f, 0.52f, 0.52f, 0.46f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.15f, 0.15f, 0.15f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.12f, 0.12f, 0.87f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.63f, 0.98f, 0.62f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.62f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.47f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.26f, 0.47f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.27f, 0.27f, 0.28f, 0.74f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.27f, 0.27f, 0.28f, 0.55f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.40f, 0.44f, 0.51f, 0.47f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.28f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.14f, 0.22f, 0.39f, 0.84f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.88f, 0.88f, 0.88f, 0.76f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.68f, 0.68f, 0.68f, 0.57f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.29f, 0.29f, 0.29f, 0.77f);
	colors[ImGuiCol_Button] = ImVec4(0.33f, 0.34f, 0.35f, 0.45f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.22f, 0.28f, 0.41f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.14f, 0.18f, 0.26f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.46f, 0.47f, 0.50f, 0.49f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.45f, 0.63f, 0.98f, 0.62f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.62f);
	colors[ImGuiCol_Separator] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.98f, 0.98f, 0.78f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.98f, 0.98f, 0.55f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.98f, 0.98f, 0.83f);
	colors[ImGuiCol_InputTextCursor] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.50f, 0.96f, 0.74f);
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.31f, 0.57f, 0.79f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.20f, 0.36f, 0.67f, 1.00f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.26f, 0.50f, 0.96f, 1.00f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.07f, 0.09f, 0.14f, 0.89f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.13f, 0.23f, 0.42f, 1.00f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.49f, 0.49f, 0.49f, 0.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.26f, 0.50f, 0.96f, 0.64f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.35f, 0.56f, 0.98f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.01f, 0.30f, 0.88f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.01f, 0.34f, 0.98f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30f, 0.32f, 0.34f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.22f, 0.23f, 0.24f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.98f, 0.98f, 0.98f, 0.06f);
	colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.50f, 0.96f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.18f, 0.39f, 0.78f, 0.83f);
	colors[ImGuiCol_TreeLines] = ImVec4(0.42f, 0.45f, 0.49f, 0.46f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.01f, 0.34f, 0.98f, 0.83f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.26f, 0.50f, 0.96f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.98f, 0.98f, 0.98f, 0.64f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.78f, 0.78f, 0.78f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.78f, 0.78f, 0.78f, 0.35f);


}
