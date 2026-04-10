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
	presetDictionary[AVAILABLE_PRESETS[0] /*Summer Bubble*/] = {
		"./models/SpeedTrees/SpeedTree.obj",
		{ "./models/SpeedTrees/singleAColor.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", },
		{ true, true, false, false, false, false, false, false }
	};
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
	presetDictionary[AVAILABLE_PRESETS[3] /*Pine 001*/] = {
		"./models/SpeedTrees/Pine001/Model.fbx",
		{ "./models/SpeedTrees/Pine001/Pine_Bark.png", "./models/SpeedTrees/Pine001/Pine_Bark.png", "./models/SpeedTrees/Pine001/Material_Leaf_Example_Combined.png", "",  "",  "",  "",  "", },
		{ true, true, true, false, false, false, false, false }
	};
}
