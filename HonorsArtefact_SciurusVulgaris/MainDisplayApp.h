#pragma once
#include "App.h"
#include "InstancedTreeRenderPass.h"
#include "ImGuiBlankRenderPass.h"
#include "Transform.h"
#include "BufferStructs.h"
#include "Light.h"


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
	char modelPath[256] = "./models/output.tree";
	float angle;

	CameraTransform cameraTransform;

	int pointToRenderCount;

	Light sun;
};

