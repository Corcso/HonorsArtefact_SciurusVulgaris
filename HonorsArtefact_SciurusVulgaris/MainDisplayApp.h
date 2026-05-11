#pragma once
#include "App.h"
#include "InstancedTreeRenderPass.h"
#include "ImGuiBlankRenderPass.h"
#include "Transform.h"
#include "BufferStructs.h"
#include "Light.h"
#include "InstancedWCPHelper.h"
#include "LightShadow_RP.h"
#include "InstancedMeshTree_RP.h"

#include "ImGuiHelpers.h"
#include "Guffer.hpp"

/// <summary>
/// <para>MainDisplayApp, Part 2 of the application</para>
/// <para>Renders the point clouds. Has settings for LOD, lighting, AA, and models loaded.</para>
/// <para>Triangle Vertex, Point Vertex, and Point Mesh shader pipelines supported.</para>
/// </summary>
class MainDisplayApp :
    public App
{
public:
	MainDisplayApp() {};

	virtual void Initialize() final;
	virtual void Frame() final;
	virtual void Shutdown() final;

	/// <summary>
	/// Render the point trees with the mesh shader pipeline
	/// </summary>
	void FrameMeshShaded();
	/// <summary>
	/// Render the point trees with the vertex shader pipeline
	/// <para>Note, Shadow map will not update, but can still be sampled from.</para>
	/// </summary>
	void FrameVertexShaded();
	/// <summary>
	/// Render the triangle trees with the vertex shader pipeline
	/// <para>Note, a lot of features are disabled here. Disable shadows when using.</para>
	/// </summary>
	void FrameMeshTrue();

private:
	/// <summary>
	/// Enum for renderer types
	/// </summary>
	enum class RendererType : int {
		MESH_TRUE, MESH_SHADED_POINTS, VERTEX_SHADED_POINTS
	};
	RendererType currentRendererType;

	InstancedTreeRenderPass pointRenderingPass;
	std::vector<size_t> descriptorSizes;

	std::vector<PointTreeMesh*> loadedPointModels; // List of loaded point models
	InstancedWorldMatrixHelper treeInstancePositions; // List of positions of point models.
	bool clearPointModelsAtFrameEnd = false;

	/// <summary>
	/// Reloads the Instance positions for both point and triangle trees from file. 
	/// <para>Used for when updating the base transform applied</para>
	/// <para>Does not update triangle mesh buffer transforms. This must be called before triangle meshes are loaded</para>
	/// </summary>
	void ReloadTreeInstancePositions();
	Transform modelBaseTransform;
	VP_Matrices treeInstanceViewProjThisAndLastFrame[2]; // This index = 0, last index = 1 (FOR TAA VELOCITY)
	const int MAX_INSTANCE_POSITIONS = 1024000;
	char modelPath[256] = "./models/output.tree";
	bool initialControlsDisplayed = false;
	bool toOpenSetupPopup = false;
	/// <summary>
	/// Loads another point tree into the list
	/// </summary>
	/// <param name="path">Path of point tree, can be duplicate</param>
	void LoadAnotherPointTree(std::string path);

	// Terrain mesh and settings
	TriListMesh* terrain;
	Image terrainTexture;
	bool terrainEnabled;

	// First person camera
	CameraTransform cameraTransform;

	int instanceCount = 1;

	// Light and shadow (depth) pass
	Light sun;
	LightShadow_RP lightShadow_RP;

	// AA controls
	bool fxaaEnabled;
	bool taaEnabled; bool taaLogarithmicColorSpace;

	// Debug controls
	bool isLODViewOn; uint32_t pointSize = 1;

	/// <summary>
	/// Renders the ImGui menu
	/// <para>Rather Large, so in its own function.</para>
	/// </summary>
	void RenderImGuiControls();

	// Triangle tree mesh, descriptors and options
	std::vector<size_t> descriptorSizesMesh;
	InstancedMeshTree_RP instancedMeshTree_RP;
	ImGuiHelpers::MultiTriListMeshLoader triangleMeshTreeLoader;
	InstancedWorldMatrixHelper treeMeshInstancePositions;

	// For Image Capture
	struct ImageCaptureRule {
		std::string name;
		std::function<void()> settings;
	};

	// For Performance & Buffer Capture.
	bool renderImGui;
	bool captureUnderway;
	float imageSequenceTimer;
	int stageImagesSaved;
	Guffer guffer;
	/// <summary>
	/// Manages the capture
	/// <para>Should be called every frame at the start</para>
	/// <para>Sets settings for each capture, performs them and keeps track of progress.</para>
	/// </summary>
	void ImageCaptureSequence();

	struct AutoShowRule {
		float length;
		std::function<void(float)> settings;
	};
	bool autoShowEnabled; int currentShowIndex; float currentShowT;
	void PlayAutoShowSequence();
};

