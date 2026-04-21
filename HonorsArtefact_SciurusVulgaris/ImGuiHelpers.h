#pragma once
#include "TriListMesh.h"
#include <functional>

namespace ImGuiHelpers {
	/// <summary>
	/// Setup ImGui with the style from (Thomet, 2026)
	/// </summary>
	void ImGuiSetupStyle();

	/// <summary>
	/// ImGui menu and holder for triangle mesh loading. 
	/// Allows triangle meshes to be loaded and their relevant textures. 
	/// </summary>
	class MultiTriListMeshLoader
	{
	public:
		MultiTriListMeshLoader();

		/// <summary>
		/// Display the Gui
		/// </summary>
		/// <param name="setupDescriptors">A function which for each triangle mesh and respective image, sets up any descriptors.</param>
		void Display(std::function<void(TriListMesh*, Image*)> setupDescriptors);

		/// <summary>
		/// Cleanup the loaded mesh (if any)
		/// </summary>
		void Cleanup();

		/// <summary>
		/// Returns the path of the loaded mesh
		/// </summary>
		/// <returns>path of the loaded mesh</returns>
		std::string WhatIsLoaded();

		/// <summary>
		/// Swaps to the preset, by name. Doesn't load it.
		/// </summary>
		/// <param name="presetName">Preset name</param>
		void SwapToPreset(std::string presetName);

		/// <summary>
		/// Loads the currently selected path(s) now. 
		/// </summary>
		/// <param name="setupDescriptors">A function which for each triangle mesh and respective image, sets up any descriptors.</param>
		void LoadNow(std::function<void(TriListMesh*, Image*)> setupDescriptors);

		/// <summary>
		/// Returns if a mesh is loaded
		/// </summary>
		bool IsMeshLoaded() { return mesh.size() > 0; }

		/// <summary>
		/// Returns the mesh vector for the loaded mesh. (Contains all meshes split by texture)
		/// </summary>
		std::vector<TriListMesh>* GetMeshVector() { return &mesh; }

		/// <summary>
		/// Returns the list of textures for the loaded mesh.
		/// </summary>
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
		/// <summary>
		/// Fills the preset dictionary with paths
		/// </summary>
		void SetupPresetDictionary();
	};
};
