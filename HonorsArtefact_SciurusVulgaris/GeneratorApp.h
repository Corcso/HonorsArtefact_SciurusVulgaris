#pragma once
#include "App.h"
#include "MeshRenderer.h"
#include "PointRenderPipeline.h"
#include "ImGuiBlankRenderPass.h"
#include "DebugPointRenderer.h"

#include "Transform.h"
#include "ImGuiHelpers.h"


/// <summary>
/// <para>Generator App, The first part of the application</para>
/// <para>Generates point cloud data from triangle meshes and allows creation of LOD Data</para>
/// </summary>
class GeneratorApp : public App
{
public:
	GeneratorApp() {};

	virtual void Initialize() final;
	virtual void Frame() final;
	virtual void Shutdown() final;
private:
	// Mesh rendering pipeline, OldUnifiedRenderer for triangle meshes
	MeshRenderer meshRenderingPipeline;
	// O.U.R for imgui, renders nothing. 
	ImGuiBlankRenderPass imguiRenderPass;
	std::vector<size_t> descriptorSizes;

	// Transform and model of loaded tri mesh
	ImGuiHelpers::MultiTriListMeshLoader treeMeshLoader;
	Transform loadedModelTransform;

	// Live output of triangle mesh rendering
	ImTextureID liveColorOut; 

	// Imgui Controls
	std::string loadStatus;
	bool isTopView = false;

	bool extractPointsAtEndOfThisFrame = false; // Will flip true when points should be extracted, and save them to file. 
	bool extractPointsConstantly = false; // Extracts and saves points every frame, just used for easy render doc capture

	// == LOD View Page ==
	DebugPointRenderer debugPointRenderer;
	/// <summary>
	/// Renders the up to 16 views of the point cloud tree with the LOD settings applied. 
	/// </summary>
	void RenderLODPagePrerequisites();
	/// <summary>
	/// Renders the ImGui Menus for LOD editing, and saving to file. 
	/// </summary>
	void RenderLODPageMenu();
	std::vector<VulkanObjectDescriptorSet> LODViewDescriptors; // A list of 16 descriptors which store each point trees descriptor. Prevents needing 16 point clouds. 
	float LODViewRotation; // Rotation of the tree. 
	float LODViewScale = 0.009f; // The default for the renderer with the default trees. 
	float LODCameraHeight = -9.5f; // Default for this summer bubble.
	std::vector<float> LODViewDistances; // List of the view distances of each display.
	int exclusivleyViewing = -1; // Exclusive viewing, allows for one level to take up entire display.
	bool isDebugCoverageViewOn; // Debug coverage renders everything as white so gaps in geometry can be easilly seen. 
	bool shuffleAtEndOfFrame = false; // Used for shuffling points so that buffer swaps happen at the end of the frame. 

};

