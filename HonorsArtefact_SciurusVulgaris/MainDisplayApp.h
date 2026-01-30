#pragma once
#include "App.h"
#include "InstancedTreeRenderPass.h"
#include "ImGuiBlankRenderPass.h"
#include "Transform.h"
#include "BufferStructs.h"
#include "Light.h"
#include "InstancedWCPHelper.h"
#include "LightShadow_RP.h"


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
	char modelPath[256] = "./models/output.tree";
	float angle;
	
	TriListMesh* terrain;
	Image terrainTexture;

	CameraTransform cameraTransform;

	int pointToRenderCount;
	int instanceCount;

	Light sun;
	LightShadow_RP lightShadow_RP;
};

