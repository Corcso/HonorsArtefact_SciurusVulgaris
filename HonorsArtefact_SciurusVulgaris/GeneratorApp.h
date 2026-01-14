#pragma once
#include "App.h"
#include "MeshRenderer.h"
#include "PointRenderPipeline.h"
#include "ImGuiBlankRenderPass.h"

#include "Transform.h"


class GeneratorApp : public App
{
public:
	GeneratorApp() {};

	virtual void Initialize() final;
	virtual void Frame() final;
	virtual void Shutdown() final;
private:
	MeshRenderer meshRenderingPipeline;
	ImGuiBlankRenderPass imguiRenderPass;
	std::vector<size_t> descriptorSizes;

	std::vector<TriListMesh> loadedModel;
	std::vector<Image> loadedImages;
	Transform loadedModelTransform;

	ImTextureID liveColorOut;

	// Imgui Controls
	std::string loadStatus;
	char modelPath[256] = "./models/SpeedTrees/SpeedTree.obj";
	bool imageActive[8]{ true, true, false, false, false, false, false, false };
	char texturePaths[8][256]{ "./models/SpeedTrees/singleAColor.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", };
	bool isTopView = false;

	bool extractPointsAtEndOfThisFrame = false; // Will flip true when points should be extracted, and save them to file. 
	bool extractPointsConstantly = false; // Extracts and saves points every frame, just used for easy render doc capture
};

