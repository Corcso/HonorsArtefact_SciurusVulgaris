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


class MainDisplayApp :
    public App
{
public:
	MainDisplayApp() {};

	virtual void Initialize() final;
	virtual void Frame() final;
	virtual void Shutdown() final;
private:
	InstancedTreeRenderPass pointRenderingPass;
	std::vector<size_t> descriptorSizes;

	PointTreeMesh* myModel;
	InstancedWCPHelper treeInstancePositions;
	InstancedWCPHelper treeInstancePositionsLastFrame;
	char modelPath[256] = "./models/output.tree";
	float angle;
	
	TriListMesh* terrain;
	Image terrainTexture;

	CameraTransform cameraTransform;

	int pointToRenderCount;
	int instanceCount = 1;

	Light sun;
	LightShadow_RP lightShadow_RP;

	bool fxaaEnabled;
	bool taaEnabled;
	void RenderImGuiControls();

	bool meshRenderOn;
	std::vector<size_t> descriptorSizesMesh;
	InstancedMeshTree_RP instancedMeshTree_RP;
	std::vector<TriListMesh> treeMesh;
	std::vector<Image> treeMeshTextures;
	char meshModelPath[256] = "./models/SpeedTrees/SpeedTree.obj";

	InstancedWCPHelper treeMeshInstancePositions;
	// For Image Capture
	struct ImageCaptureRule {
		std::string name;
		std::function<void()> settings;
	};

	bool renderImGui;
	bool captureUnderway;
	float imageSequenceTimer;
	int stageImagesSaved;
	void ImageCaptureSequence();
};

