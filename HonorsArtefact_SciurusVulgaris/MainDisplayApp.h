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


class MainDisplayApp :
    public App
{
public:
	MainDisplayApp() {};

	virtual void Initialize() final;
	virtual void Frame() final;
	virtual void Shutdown() final;

	void FrameMeshShaded();
	void FrameVertexShaded();
	void FrameMeshTrue();

private:
	enum class RendererType : int {
		MESH_TRUE, MESH_SHADED_POINTS, VERTEX_SHADED_POINTS
	};
	RendererType currentRendererType;

	InstancedTreeRenderPass pointRenderingPass;
	std::vector<size_t> descriptorSizes;

	std::vector<PointTreeMesh*> loadedPointModels;
	InstancedWorldMatrixHelper treeInstancePositions;
	VP_Matrices treeInstanceViewProjThisAndLastFrame[2]; // This index = 0, last index = 1
	const int MAX_INSTANCE_POSITIONS = 1024000;
	char modelPath[256] = "./models/output.tree";
	float angle;
	bool initialControlsDisplayed = false;
	bool toOpenSetupPopup = false;
	void LoadAnotherPointTree(std::string path);

	TriListMesh* terrain;
	Image terrainTexture;
	bool terrainEnabled;

	CameraTransform cameraTransform;

	int pointToRenderCount;
	int instanceCount = 1;

	Light sun;
	LightShadow_RP lightShadow_RP;

	bool fxaaEnabled;
	bool taaEnabled; bool taaLogarithmicColorSpace;
	void RenderImGuiControls();

	bool meshRenderOn;
	std::vector<size_t> descriptorSizesMesh;
	InstancedMeshTree_RP instancedMeshTree_RP;
	ImGuiHelpers::MultiTriListMeshLoader triangleMeshTreeLoader;

	InstancedWorldMatrixHelper treeMeshInstancePositions;
	// For Image Capture
	struct ImageCaptureRule {
		std::string name;
		std::function<void()> settings;
	};

	bool renderImGui;
	bool captureUnderway;
	float imageSequenceTimer;
	int stageImagesSaved;
	Guffer guffer;
	void ImageCaptureSequence();
};

