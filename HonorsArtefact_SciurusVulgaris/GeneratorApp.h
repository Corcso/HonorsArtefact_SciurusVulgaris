#pragma once
#include "App.h"
#include "MeshRenderer.h"
#include "PointRenderPipeline.h"
#include "ImGuiBlankRenderPass.h"
#include "DebugPointRenderer.h"

#include "Transform.h"
#include "ImGuiHelpers.h"

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

	ImGuiHelpers::MultiTriListMeshLoader treeMeshLoader;
	Transform loadedModelTransform;

	ImTextureID liveColorOut;

	// Imgui Controls
	std::string loadStatus;
	bool isTopView = false;

	bool extractPointsAtEndOfThisFrame = false; // Will flip true when points should be extracted, and save them to file. 
	bool extractPointsConstantly = false; // Extracts and saves points every frame, just used for easy render doc capture

	// == LOD View Page ==
	DebugPointRenderer debugPointRenderer;
	void RenderLODPagePrerequisites();
	void RenderLODPageMenu();
	std::vector<VulkanObjectDescriptorSet> LODViewDescriptors;
	float LODViewRotation;
	float LODViewScale = 0.009f; // The default for the renderer with the default trees. 
	float LODCameraHeight = -9.5f; // Default for this summer bubble.
	std::vector<float> LODViewDistances;
	int exclusivleyViewing = -1;
	bool isDebugCoverageViewOn;
	bool shuffleAtEndOfFrame = false; // Used for shuffling points so that buffer swaps happen at the end of the frame. 

};

