#pragma once
#include "App.h"
#include "InstancedTreeRenderPass.h"
#include "ImGuiBlankRenderPass.h"
#include "Transform.h"
#include "BufferStructs.h"


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

	PointMesh* myModel;
	char modelPath[256] = "./models/output.fbx";
	float angle;

	CameraTransform cameraTransform;

	int pointToRenderCount;
};

