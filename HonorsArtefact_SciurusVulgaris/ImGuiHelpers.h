#pragma once
#include "TriListMesh.h"
#include <functional>

namespace ImGuiHelpers {
	class MultiTriListMeshLoader
	{
	public:
		MultiTriListMeshLoader();

		void Display(std::function<void(TriListMesh*, Image*)> setupDescriptors);

		void Cleanup();

		std::string WhatIsLoaded();
		void SwapToPreset(std::string presetName);
		void LoadNow(std::function<void(TriListMesh*, Image*)> setupDescriptors);

		bool IsMeshLoaded() { return mesh.size() > 0; }

		std::vector<TriListMesh>* GetMeshVector() { return &mesh; }
		std::vector<Image>* GetTexturesVector() { return &textures; }
	private:
		std::string meshPath;
		std::vector<std::string> texturePaths;
		std::vector<uint8_t> texturesEnabled;

		std::string meshPathCurrentlyLoaded;
		std::vector<TriListMesh> mesh;
		std::vector<Image> textures;

		// Presets 
		int currentlyChosenPresetIndex;
		struct TriListMeshPathPreset {
			std::string meshPath;
			std::vector<std::string> texturePaths;
			std::vector<uint8_t> texturesEnabled;
		};
		std::map<std::string, TriListMeshPathPreset> presetDictionary;

		static const uint16_t AVAILABLE_PRESET_COUNT = 5;
		const char* AVAILABLE_PRESETS[AVAILABLE_PRESET_COUNT]{
			"Summer Bubble", "Autumn Bubble", "512 Triangle Plane", "Pine 001", "Conifer 001"
		};

		void SetupPresetDictionary();
	};
};
