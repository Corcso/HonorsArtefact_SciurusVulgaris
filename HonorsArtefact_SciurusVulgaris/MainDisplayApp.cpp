#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>
#include "csv2.hpp"

void MainDisplayApp::Initialize() {
	// Create point and tree Render passes
	pointRenderingPass.CreateAll(); 
	instancedMeshTree_RP.CreateAll();
	descriptorSizes = { 0, sizeof(HMM_Mat4) * MAX_INSTANCE_POSITIONS, sizeof(InstancingInfo), sizeof(MeshletInfo), sizeof(LODDataBuffer), sizeof(TAAInfo), sizeof(VP_Matrices) * 2};
	descriptorSizesMesh = { sizeof(HMM_Mat4) * MAX_INSTANCE_POSITIONS, 0, sizeof(VP_Matrices) * 2 };
	
	// Setup camera above scene
	cameraTransform.speed = 0.5f;
	cameraTransform.position = HMM_V3(0, 30, 0);
	cameraTransform.euler = HMM_V3(-45, 0, 0);
	
	// Setup sun and shadow render pass
	lightShadow_RP.CreateAll();
	sun.SetName("Sun");
	sun.CreateShadowResources(lightShadow_RP.GetRenderPass());

	// Setup terrain
	terrain = new TriListMesh();
	// ChinaValley map from (Mustoe-Playfair, 2026)
	terrain->LoadFile("./models/ChinaValley/ChinaValley.fbx", 0);
	terrain->CopyPointsToVRAM();
	terrainTexture.CreateAndLoadImageFromFile("./models/ChinaValley/Colour.png", VK_IMAGE_USAGE_SAMPLED_BIT);
	terrainTexture.CreateImageView();
	size_t sizes[] = { sizeof(WCP_Matrices) * 2, 0, sizeof(TAAInfo)};
	terrain->GetDescriptorSet()->Create(pointRenderingPass.GetMeshTraditionalDescriptorSetLayout(), pointRenderingPass.GetMeshTraditionalDescriptorSetLayoutInfo(), sizes);
	terrain->GetDescriptorSet()->UpdateImageSampler(1, &terrainTexture, Graphics::GetBasicLinearSampler());

	// Setup instance positions, with defaults for default set of trees and terrain
	modelBaseTransform.position = HMM_V3(0, 0.03, 0);
	modelBaseTransform.scale = HMM_V3(0.009, 0.009, 0.009);
	modelBaseTransform.UpdateMatrix();
	ReloadTreeInstancePositions();

	// Set shadow images for the geometry paint pass
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateImageSampler(4, Graphics::GetNoShadowMapImage(), Graphics::GetBasicNearestSampler());
	pointRenderingPass.GetQuadDescriptorSet()->UpdateImageSampler(4, sun.GetShadowImage(), Graphics::GetBasicNearestSampler());

	captureUnderway = false;
	renderImGui = true;

	// Default to mesh shaded
	currentRendererType = RendererType::MESH_SHADED_POINTS;

	// If we are doing a full capture run, load the relevant models
	if (Graphics::IsFullCaptureRunActive()) {
		// Use default paths
		// Load Point Mesh
		std::cout << "Loading Point Tree (Summer Bubble)...\n";
		{
			loadedPointModels.clear();

			loadedPointModels.push_back(new PointTreeMesh());

			loadedPointModels[0]->LoadFromTreeFile("./model/output512.tree");
			descriptorSizes[0] = loadedPointModels[0]->GetPointsArraySize(true);
			loadedPointModels[0]->CreateDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
			loadedPointModels[0]->CreateShadowDescriptorSet(lightShadow_RP.GetShadowSetLayout(), lightShadow_RP.GetShadowSetLayoutInfo(), descriptorSizes.data());
			loadedPointModels[0]->CopyPointsToVRAMMeshBuffer(0);
			loadedPointModels[0]->CopyPointsToVRAM();

			// Copy instance positions
			loadedPointModels[0]->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
			loadedPointModels[0]->GetShadowDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());


			MeshletInfo meshletInfo{ loadedPointModels[0]->GetMeshletCount() };
			loadedPointModels[0]->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
		}
		// Load mesh
		std::cout << "Loading Full Tree (Summer Bubble)... This can take some time.\n";
		triangleMeshTreeLoader.LoadNow([&](TriListMesh* mesh, Image* texture) {
			mesh->CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
			mesh->GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
			mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, Graphics::GetBasicLinearSampler());
			});

		// Begin Capture
		imageSequenceTimer = 0;
		captureUnderway = true;
		stageImagesSaved = -1;
	}
}

void MainDisplayApp::Frame() {
	ImageCaptureSequence(); // Apply settings for capture 

	cameraTransform.CaptureControls();
	
	switch (currentRendererType) {
	case RendererType::MESH_TRUE:
		FrameMeshTrue();
		break;
	case RendererType::MESH_SHADED_POINTS:
		FrameMeshShaded();
		break;
	case RendererType::VERTEX_SHADED_POINTS:
		FrameVertexShaded();
		break;
	}
}

void MainDisplayApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	for (auto& model : loadedPointModels) {
		if (model != nullptr) {
			delete model;
		}
	}
	loadedPointModels.clear();
	triangleMeshTreeLoader.Cleanup();
	delete terrain; // Always loaded
	terrainTexture.Destroy();
	sun.ShutdownShadowResources();
	pointRenderingPass.Shutdown();
	instancedMeshTree_RP.Shutdown();
	lightShadow_RP.Shutdown();
}

void MainDisplayApp::FrameMeshShaded()
{
	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	Graphics::BeginRender();

	// Set LOD & instance Data based on model
	if (loadedPointModels.size() > 0 && loadedPointModels[0] != nullptr) {
		for (int l = 0; l < loadedPointModels[0]->randomLevelsLODPointCount.size(); l++) {
			lodData.maxVertexLevels[l][0] = loadedPointModels[0]->randomLevelsLODPointCount[l];
		}
		lodData.maxLevel = loadedPointModels[0]->randomLevelsLODPointCount.size();
		lodData.continousDecay = loadedPointModels[0]->continousLOD_decay;
		lodData.continousStart = loadedPointModels[0]->continousLOD_start;
		lodData.continousShallowness = loadedPointModels[0]->continousLOD_shallowness;
		lodData.lodType = static_cast<int>(loadedPointModels[0]->levelOfDetailType);
	}

	// Perform shadow pass
	if (sun.IsShadowEnabled()) {
		Graphics::PushMetricRange("Shadow Map Render");
		lightShadow_RP.BeginRender(&sun);
		uint32_t index = 0;
		for (auto& model : loadedPointModels) {
			if (model != nullptr) {
				// Update LOD with y=0 (where the shadow renders from)
				lodData.cameraPosition = HMM_V4(cameraTransform.position.X, 0, cameraTransform.position.Z, 1);

				// Update all buffers
				VP_Matrices shadowMap = { sun.GetViewMatrix(HMM_V3(cameraTransform.position.X, 0.0f, cameraTransform.position.Z)), sun.GetProjectionMatrix(150, 150, 150) };
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(6, &shadowMap);
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(4, &lodData);

				instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount(), static_cast<uint32_t>(loadedPointModels.size()), index };
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

				lightShadow_RP.RenderPointTree(model, instancingInfo);
			}
			index++;
		}
		lightShadow_RP.EndRender();
		Graphics::PopMetricRange();
	}
	// Render point trees to GBuffers
	Graphics::PushMetricRange("Render Point Trees");

	pointRenderingPass.UpdateCameraInfoForSkybox(cameraTransform);
	pointRenderingPass.BeginRenderMeshShade(HMM_V4(0, 0, 0, 1));

	uint32_t index = 0;
	for (auto& model : loadedPointModels) {
		if (model != nullptr) {
			// LOD now with Y value
			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeInstanceViewProjThisAndLastFrame[0] = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(50 * HMM_DegToRad, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };
			model->GetDescriptorSet()->UpdateUniformBufferData(6, &treeInstanceViewProjThisAndLastFrame);
			model->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount(), static_cast<uint32_t>(loadedPointModels.size()), index };
			model->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

			pointRenderingPass.UpdateTAADescriptor(model->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

			pointRenderingPass.RenderPointTreeViaMeshShader(model, instancingInfo);
		}
		index++;
	}



	// Terrain render
	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(0, 0, 0)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(50 * HMM_DegToRad, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000)
		},
		{
		HMM_Translate(HMM_V3(0, 0, 0)),
		treeInstanceViewProjThisAndLastFrame[1].camera,
		treeInstanceViewProjThisAndLastFrame[1].projection
		}
	};
	terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
	pointRenderingPass.UpdateTAADescriptor(terrain->GetDescriptorSet(), 2, taaEnabled, taaLogarithmicColorSpace);
	Graphics::PopMetricRange();
	Graphics::PushMetricRange("Terrain Mesh Render");
	if (terrainEnabled) {
		pointRenderingPass.SwitchToTraditionalMeshPipeline();
		pointRenderingPass.RenderTraditionalMesh(terrain);
	}
	pointRenderingPass.EndRender();
	Graphics::PopMetricRange();

	// Render ImGui Menu (please note, imgui always renders last, this is just where the windows are called)
	RenderImGuiControls();

	// Paint deferred pass, and AA.
	Light::BufferStruct rawSunData = sun.GetBufferData();
	pointRenderingPass.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);

	Graphics::PushMetricRange("Colour Deferred");
	pointRenderingPass.ExecuteSecondRender();
	pointRenderingPass.EndSecondRender();
	Graphics::PopMetricRange();
	Graphics::PushMetricRange("Anti Aliasing");
	pointRenderingPass.ExecuteTAARender(taaEnabled, taaLogarithmicColorSpace);
	pointRenderingPass.EndTAARender();

	pointRenderingPass.ExecuteFXAARender(fxaaEnabled);
	Graphics::PopMetricRange();
	Graphics::FinishImGuiRender();
	pointRenderingPass.EndFXAARender();
	Graphics::EndRender();

	// Store last VP matrices for TAA
	treeInstanceViewProjThisAndLastFrame[1] = treeInstanceViewProjThisAndLastFrame[0];
}

void MainDisplayApp::FrameVertexShaded()
{
	// Very similar to Mesh shaded. Without shadow depth pass.
	// Read above comments for more detail. 

	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	Graphics::BeginRender();

	Graphics::PushMetricRange("Render Point Trees");
	pointRenderingPass.BeginRenderVertexShade(HMM_V4(0, 0, 0, 1));

	uint32_t index = 0;
	for (auto& model : loadedPointModels) {
		if (model != nullptr) {

			// LOD data always taken from the first model only, as multiple LOD settings not supported. 
			// If we are in the loop there has to be 1 model loaded. 
			if (loadedPointModels[0] != nullptr) {
				for (int l = 0; l < loadedPointModels[0]->randomLevelsLODPointCount.size(); l++) {
					lodData.maxVertexLevels[l][0] = loadedPointModels[0]->randomLevelsLODPointCount[l];
				}
				lodData.maxLevel = loadedPointModels[0]->randomLevelsLODPointCount.size();
				lodData.continousDecay = loadedPointModels[0]->continousLOD_decay;
				lodData.continousStart = loadedPointModels[0]->continousLOD_start;
				lodData.continousShallowness = loadedPointModels[0]->continousLOD_shallowness;
				lodData.lodType = static_cast<int>(loadedPointModels[0]->levelOfDetailType);
			}

			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeInstanceViewProjThisAndLastFrame[0] = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(50 * HMM_DegToRad, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };
			model->GetDescriptorSet()->UpdateUniformBufferData(6, &treeInstanceViewProjThisAndLastFrame);
			model->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount() , static_cast<uint32_t>(loadedPointModels.size()), index };
			model->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

			pointRenderingPass.UpdateTAADescriptor(model->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

			pointRenderingPass.RenderPointTree(model, instancingInfo);
		}
		index++;
	}

	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(0, 0, 0)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(50 * HMM_DegToRad, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000)
		},
		{
		HMM_Translate(HMM_V3(0, 0, 0)),
		treeInstanceViewProjThisAndLastFrame[1].camera,
		treeInstanceViewProjThisAndLastFrame[1].projection
		}
	};
	terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
	pointRenderingPass.UpdateTAADescriptor(terrain->GetDescriptorSet(), 2, taaEnabled, taaLogarithmicColorSpace);
	Graphics::PopMetricRange();
	Graphics::PushMetricRange("Terrain Mesh Render");
	if (terrainEnabled) {
		pointRenderingPass.SwitchToTraditionalMeshPipeline();
		pointRenderingPass.RenderTraditionalMesh(terrain);
	}
	pointRenderingPass.EndRender();
	Graphics::PopMetricRange();

	// Imgui not placed here, its always rendered last.
	RenderImGuiControls();

	Light::BufferStruct rawSunData = sun.GetBufferData();
	pointRenderingPass.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);

	Graphics::PushMetricRange("Colour Deferred");
	pointRenderingPass.ExecuteSecondRender();
	pointRenderingPass.EndSecondRender();
	Graphics::PopMetricRange();
	Graphics::PushMetricRange("Anti Aliasing");
	pointRenderingPass.ExecuteTAARender(taaEnabled, taaLogarithmicColorSpace);
	pointRenderingPass.EndTAARender();

	pointRenderingPass.ExecuteFXAARender(fxaaEnabled);
	Graphics::PopMetricRange();
	Graphics::FinishImGuiRender();
	pointRenderingPass.EndFXAARender();
	Graphics::EndRender();

	treeInstanceViewProjThisAndLastFrame[1] = treeInstanceViewProjThisAndLastFrame[0];
}

void MainDisplayApp::FrameMeshTrue()
{
	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	Graphics::BeginRender();
	
	// Render logic
	instancedMeshTree_RP.BeginRender();
	Graphics::PushMetricRange("Render Mesh Trees");
	if (triangleMeshTreeLoader.GetMeshVector()->size() > 0) {
		lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

		for (int i = 0; i < triangleMeshTreeLoader.GetMeshVector()->size(); i++) {

			InstancingInfo instancingInfo{ instanceCount, 0 };
			VP_Matrices viewCamMatrices = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(50 * HMM_DegToRad, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };

			(*triangleMeshTreeLoader.GetMeshVector())[i].GetDescriptorSet()->UpdateUniformBufferData(2, &viewCamMatrices); // Only 1/2 of buffer update but second half not used.
			instancedMeshTree_RP.RenderMeshTree(&(*triangleMeshTreeLoader.GetMeshVector())[i], instancingInfo);
		}
	}
	Graphics::PopMetricRange();

	instancedMeshTree_RP.EndRender();

	// Imgui not placed here, its always rendered last.
	RenderImGuiControls();

	// Still deferred, but no post processing.

	Light::BufferStruct rawSunData = sun.GetBufferData();
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);

	Graphics::PushMetricRange("Colour Deferred");
	instancedMeshTree_RP.ExecuteSecondRender();
	Graphics::FinishImGuiRender();
	instancedMeshTree_RP.EndSecondRender();

	Graphics::EndRender();
}

void MainDisplayApp::ReloadTreeInstancePositions()
{
	// Load the 1,024,000 position files and replace the existing data. 
	// Then apply the new transform on top.
	vkDeviceWaitIdle(Graphics::GetVkDevice());
	treeInstancePositions.LoadFromFile("./models/ChinaValley/ChinaValleyLocations1024K.obj");
	uint64_t chosenSeed = treeInstancePositions.ApplyRandomRotation(Graphics::IsFullCaptureRunActive() ? 1 : 0);
	treeInstancePositions.ApplyAlternateTransform(modelBaseTransform.matrix);

	treeMeshInstancePositions.LoadFromFile("./models/ChinaValley/ChinaValleyLocations1024K.obj");
	treeMeshInstancePositions.ApplyRandomRotation(chosenSeed);
	treeMeshInstancePositions.ApplyAlternateTransform(modelBaseTransform.matrix);

	// Update point trees buffers
	for (auto& model : loadedPointModels) {
		if (model == nullptr) continue;
		model->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
		model->GetShadowDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
	}
	// Note: doesn't update triangle trees. Must be called before load.
}

void MainDisplayApp::LoadAnotherPointTree(std::string path)
{
	loadedPointModels.push_back(new PointTreeMesh());

	loadedPointModels[loadedPointModels.size() - 1]->LoadFromTreeFile(path);

	// Load model to GPU and create descriptor sets
	descriptorSizes[0] = loadedPointModels[loadedPointModels.size() - 1]->GetPointsArraySize(true);
	loadedPointModels[loadedPointModels.size() - 1]->CreateDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
	loadedPointModels[loadedPointModels.size() - 1]->CreateShadowDescriptorSet(lightShadow_RP.GetShadowSetLayout(), lightShadow_RP.GetShadowSetLayoutInfo(), descriptorSizes.data());
	loadedPointModels[loadedPointModels.size() - 1]->CopyPointsToVRAMMeshBuffer(0);
	loadedPointModels[loadedPointModels.size() - 1]->CopyPointsToVRAM();

	// Copy instance positions
	loadedPointModels[loadedPointModels.size() - 1]->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
	loadedPointModels[loadedPointModels.size() - 1]->GetShadowDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());

	// Setup meshlet info
	MeshletInfo meshletInfo{ loadedPointModels[loadedPointModels.size() - 1]->GetMeshletCount() };
	loadedPointModels[loadedPointModels.size() - 1]->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
}

void MainDisplayApp::RenderImGuiControls()
{
	if (!renderImGui) return;
	// == Point model selection & transform window == 
	ImGui::Begin("Point Model Selection");
	ImGui::InputText("Model Path", modelPath, 256);
	if (ImGui::Button("Load Another")) {
		LoadAnotherPointTree(modelPath);
	}
	ImGui::Text("%i Models Loaded", loadedPointModels.size());
	if (ImGui::Button("Clear All")) {
		loadedPointModels.clear();
	}
	ImGui::Text("Tree Transform");
	modelBaseTransform.DisplayController();
	if (ImGui::Button("Update Transform")) {
		modelBaseTransform.UpdateMatrix();
		ReloadTreeInstancePositions();
	}
	if (triangleMeshTreeLoader.GetMeshVector()->size() > 0) {
		ImGui::Text("Heads up! The transforms cannot\nupdate for the triangle model.\nInstead, please change them,\nthen load the model!");
	}
	ImGui::End();
	// == Metrics window == 
	ImGui::Begin("Meterage");
	ImGui::Text("FPS %i", Clock::GetFPS());
	ImGui::Text("MS Render %f", Clock::DeltaTime() * 1000);
#ifdef NV_PERF_METER
	if (ImGui::Button("NV Perf Run")) {
		Graphics::nvperf_InitiateReport("Manual Trigger");
	}
	ImGui::Text(("Saved to" + Graphics::nfperf_GetLastReportDir()).c_str());
#endif
	if (ImGui::Button("Save Swap Chain")) {
		Graphics::SaveSwapChainImageToFile("./swapchainout.bmp");
	}
	if (ImGui::Button("Run Save Sequence")) {
		imageSequenceTimer = 0;
		captureUnderway = true;
		stageImagesSaved = -1;
	}
	ImGui::End();
	// == Triangle model loader (transform shared with point) == 
	ImGui::Begin("Triangle Model");
	triangleMeshTreeLoader.Display([&](TriListMesh* mesh, Image* texture) {
		mesh->CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
		mesh->GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
		mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, Graphics::GetBasicLinearSampler());
		});
	ImGui::End();
	// == Render method & options window == 
	ImGui::Begin("Render Method");
	const char* items[] = { "True Mesh", "Mesh Shaded Points", "Vertex Shaded Points" };
	ImGui::Combo("Renderer", reinterpret_cast<int*>(&currentRendererType), items, 3);
	if((currentRendererType == RendererType::MESH_SHADED_POINTS || currentRendererType == RendererType::VERTEX_SHADED_POINTS) && loadedPointModels.size() == 0) ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "No Point Mesh Tree Loaded");
	else if (currentRendererType == RendererType::MESH_TRUE && !triangleMeshTreeLoader.IsMeshLoaded()) ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "No Triangle Mesh Tree Loaded");

	if (currentRendererType == RendererType::MESH_SHADED_POINTS || currentRendererType == RendererType::VERTEX_SHADED_POINTS) ImGui::Checkbox("FXAA", &fxaaEnabled);
	else ImGui::Text("FXAA Not Available");
	if (currentRendererType == RendererType::MESH_SHADED_POINTS || currentRendererType == RendererType::VERTEX_SHADED_POINTS) ImGui::Checkbox("TAA", &taaEnabled);
	else ImGui::Text("TAA Not Available");
	if ((currentRendererType == RendererType::MESH_SHADED_POINTS || currentRendererType == RendererType::VERTEX_SHADED_POINTS) && taaEnabled) ImGui::Checkbox("Logarithmic Colour Space", &taaLogarithmicColorSpace);

	if ((loadedPointModels.size() > 0 && loadedPointModels[0] != nullptr) || triangleMeshTreeLoader.IsMeshLoaded()) ImGui::DragInt("N Instances", &instanceCount, 64, 0, HMM_MIN(treeInstancePositions.matrices.size(), MAX_INSTANCE_POSITIONS),"%d", ImGuiSliderFlags_Logarithmic);
	else ImGui::Text("Please load a model to instance items");

	if (currentRendererType == RendererType::MESH_SHADED_POINTS || currentRendererType == RendererType::VERTEX_SHADED_POINTS) ImGui::Checkbox("Terrain", &terrainEnabled);
	else ImGui::Text("Terrain Not Available");
	ImGui::End();
	// == LOD editor == 
	ImGui::Begin("Live LOD Edits");
	ImGui::Text("Please note:\nPoint count rounded up\nto nearest 128.");

	// Continuous
	if ((loadedPointModels.size() > 0 && loadedPointModels[0] != nullptr) && loadedPointModels[0]->levelOfDetailType == PointTreeMesh::LODType::CONTINUOUS) {
		if (ImGui::Button("Swap to Discrete")) loadedPointModels[0]->levelOfDetailType = PointTreeMesh::LODType::RANDOM_LEVELS;
		ImGui::DragFloat("Level 0 Points", &loadedPointModels[0]->continousLOD_start, 128, 0, loadedPointModels[0]->points.size());
		ImGui::DragFloat("Shallowness", &loadedPointModels[0]->continousLOD_shallowness, 1, 0, 100);
		ImGui::DragFloat("Decay", &loadedPointModels[0]->continousLOD_decay, 0.1f, 1, 5);
	}

	// Discrete
	if ((loadedPointModels.size() > 0 && loadedPointModels[0] != nullptr) && loadedPointModels[0]->levelOfDetailType == PointTreeMesh::LODType::RANDOM_LEVELS) {
		if (ImGui::Button("Swap to Continuous")) loadedPointModels[0]->levelOfDetailType = PointTreeMesh::LODType::CONTINUOUS;
		for (int i = 0; i < loadedPointModels[0]->randomLevelsLODPointCount.size(); i++) {
			unsigned int max = loadedPointModels[0]->points.size();
			ImGui::DragScalar(std::to_string(i).c_str(), ImGuiDataType_U32, &(loadedPointModels[0]->randomLevelsLODPointCount[i]), 1, 0, &max);
		}
		if (ImGui::Button("-") && loadedPointModels[0]->randomLevelsLODPointCount.size() > 0) {
			loadedPointModels[0]->randomLevelsLODPointCount.pop_back();
		}
		if (ImGui::Button("+") && loadedPointModels[0]->randomLevelsLODPointCount.size() < 16) {
			if (loadedPointModels[0]->randomLevelsLODPointCount.size() < 1) {
				loadedPointModels[0]->randomLevelsLODPointCount.push_back(
					loadedPointModels[0]->points.size()
				);
			}
			else {
				loadedPointModels[0]->randomLevelsLODPointCount.push_back(
					loadedPointModels[0]->randomLevelsLODPointCount[loadedPointModels[0]->randomLevelsLODPointCount.size() - 1] / 2
				);
			}
		}
	}

	ImGui::End();

	// == Camera position viewer == 
	ImGui::Begin("Camera Position Information");
	ImGui::Text("Pos: %f %f %f", cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z);
	ImGui::Text("Rot: %f %f %f", cameraTransform.euler.X, cameraTransform.euler.Y, cameraTransform.euler.Z);
	ImGui::End();

	// == lighting window == 
	ImGui::Begin("Lighting");
	ImGui::Checkbox("Enable Skybox", &pointRenderingPass.enableSkybox);
	sun.RenderImGuiMenu(false);
	ImGui::End();
	// == Popups == 
	if (ImGui::BeginPopupModal("Setup Automatically?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Would you like to automatically set up the scene?");
		ImGui::Separator();
		ImGui::Text("This will load three models, enable shadows, configure lighting, and enable terrain.\n 64,000 trees will be displayed. With Skybox Enabled");
		ImGui::Text("Please note: You must have output, outputPine1 and outputConifer1 trees at ./models/,\n otherwise this may crash!");

		if (ImGui::Button("Set up", ImVec2(120, 0))) { 
			LoadAnotherPointTree("./models/output.tree");
			LoadAnotherPointTree("./models/outputPine1.tree");
			LoadAnotherPointTree("./models/outputConifer1.tree");
			instanceCount = 64000;
			terrainEnabled = true;
			sun.SetShadowEnabled(true);
			sun.SetDirection(HMM_V3(0.5, -1, 0));
			pointRenderingPass.enableSkybox = true;

			sun.SetAmbientColor(HMM_V3(240.0f / 255.0f, 240.0f / 255.0f, 255.0f / 255.0f));
			sun.SetAmbientIntensity(0.3f);
			sun.SetColor(HMM_V3(255.0f / 255.0f, 255.0f / 255.0f, 240.0f / 255.0f));
			sun.SetIntensity(1.0f);

			ImGui::CloseCurrentPopup(); 
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }

		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Controls", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("See control scheme below.");
		ImGui::Separator();
		ImGui::Text("WASD - Lateral movement");
		ImGui::Text("QE - Vertical movement");
		ImGui::Text("P - Lock mouse for first person camera");
		ImGui::Text("Z/X - Slow and Speed up movement");

		if (ImGui::Button("Gotcha", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); toOpenSetupPopup = true; }

		ImGui::EndPopup();
	}
	// First display controls, when closed, display setup 
	if (!initialControlsDisplayed) {
		ImGui::OpenPopup("Controls");
		initialControlsDisplayed = true;
	}
	if (toOpenSetupPopup) {
		ImGui::OpenPopup("Setup Automatically?");
		toOpenSetupPopup = false;
	}
}

void MainDisplayApp::ImageCaptureSequence()
{
	if (!captureUnderway) return;
	// Set out capture rules, these are named captures which perform the lambda each frame to setup options for the capture. 
	// Last capture's options are not wiped, so the full list of options isn't needed every time. 
	std::vector<ImageCaptureRule> rules{
		// STANDARD MESH SHADER POINTS
		{"DownTheValley4kMeshShade", [&]() {
			cameraTransform.position = HMM_V3(24.5, -15.2, -107.21);
			cameraTransform.euler = HMM_V3(4.5, 23.5, 0);
			instanceCount = 4000;
			currentRendererType = RendererType::MESH_SHADED_POINTS;
			fxaaEnabled = false;
			taaEnabled = false;
			terrainEnabled = false;
			sun.SetShadowEnabled(false);
			pointRenderingPass.enableSkybox = false;
			loadedPointModels[0]->continousLOD_start = 512.0f;
			loadedPointModels[0]->continousLOD_shallowness = 12.5f;
			loadedPointModels[0]->continousLOD_decay = 1.5f;
		} },
		//{"DownTheValley16kMeshShade", [&]() {
		//	instanceCount = 16000;
		//} },
		//{"DownTheValley32kMeshShade", [&]() {
		//	instanceCount = 32000;
		//} },
		//{"DownTheValley64kMeshShade", [&]() {
		//	instanceCount = 64000;
		//} },
		//{"DownTheValley128kMeshShade", [&]() {
		//	instanceCount = 128000;
		//} },
		//{"DownTheValley256kMeshShade", [&]() {
		//	instanceCount = 256000;
		//} },
		//{"DownTheValley512kMeshShade", [&]() {
		//	instanceCount = 512000;
		//} },
		//{"DownTheValley1024kMeshShade", [&]() {
		//	instanceCount = 1024000;
		//} },
		//// STANDARD VERTEX SHADER POINTS
		//{"DownTheValley4kVertShade", [&]() {
		//	instanceCount = 4000;
		//	currentRendererType = RendererType::VERTEX_SHADED_POINTS;
		//} },
		//{"DownTheValley16kVertShade", [&]() {
		//	instanceCount = 16000;
		//} },
		//{"DownTheValley32kVertShade", [&]() {
		//	instanceCount = 32000;
		//} },
		//{"DownTheValley64kVertShade", [&]() {
		//	instanceCount = 64000;
		//} },
		///*{"DownTheValley128kVertShade", [&]() {TOO SLOW ON 2060
		//	instanceCount = 128000;
		//} },
		//{"DownTheValley256kVertShade", [&]() {TOO SLOW ON 2060
		//	instanceCount = 256000;
		//} },
		//{"DownTheValley512kVertShade", [&]() {TOO SLOW ON 2060
		//	instanceCount = 512000;
		//} },
		//{"DownTheValley1024kVertShade", [&]() {TOO SLOW ON 2060
		//	instanceCount = 1024000;
		//} },*/
		//// STANDARD FULL MESH TRUE
		//{"DownTheValley4True", [&]() {
		//	instanceCount = 4;
		//	currentRendererType = RendererType::MESH_TRUE;
		//} },
		//{"DownTheValley16True", [&]() {
		//	instanceCount = 16;
		//} },
		//{"DownTheValley32True", [&]() {
		//	instanceCount = 32;
		//} },
		//{"DownTheValley64True", [&]() {
		//	instanceCount = 64;
		//} },
		//{"DownTheValley128True", [&]() {
		//	instanceCount = 128;
		//} },
		//{"DownTheValley256True", [&]() {
		//	instanceCount = 256;
		//} },
		/*
		{"DownTheValley512True", [&]() { TOO SLOW ON 2060
			instanceCount = 512;
		} },
		{"DownTheValley1024True", [&]() { TOO SLOW ON 2060
			instanceCount = 1024;
		} }*/
		// VISUAL COMPARISON
		{"RenderComparisonCloseTrue", [&]() {
			currentRendererType = RendererType::MESH_TRUE;
			loadedPointModels[0]->continousLOD_start = loadedPointModels[0]->points.size(); // Disable LOD
			loadedPointModels[0]->continousLOD_decay = 1.0f;
			instanceCount = 100; // To get the two trees next to eachother
			cameraTransform.position = HMM_V3(-29.2, -11.2, -29.9);
			cameraTransform.euler = HMM_V3(-17.3, 518.7, 0);
		} },
		{"RenderComparisonClosePoint", [&]() {
			currentRendererType = RendererType::MESH_SHADED_POINTS;
		} },
		{"RenderComparisonFarTrue", [&]() {
			currentRendererType = RendererType::MESH_TRUE;
			cameraTransform.position = HMM_V3(-27, -9.3, -24.1);
		} },
		{"RenderComparisonFarPoint", [&]() {
			currentRendererType = RendererType::MESH_SHADED_POINTS;
		} },
		// 512 Comparisons | Triangles vs Points 
		//{ "DownTheValley4kTrue512", [&]() {
		//	cameraTransform.position = HMM_V3(24.5, -15.2, -107.21);
		//	cameraTransform.euler = HMM_V3(4.5, 23.5, 0);
		//	instanceCount = 4000;
		//	currentRendererType = RendererType::MESH_TRUE;

		//	if (triangleMeshTreeLoader.WhatIsLoaded() != "./models/Testing/512TrianglePlane.obj") {
		//		triangleMeshTreeLoader.SwapToPreset("512 Triangle Plane");
		//		triangleMeshTreeLoader.LoadNow([&](TriListMesh* mesh, Image* texture) {
		//			mesh->CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
		//			mesh->GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
		//			mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, Graphics::GetBasicLinearSampler());
		//			});
		//	}
		//} },
		//{ "DownTheValley16kTrue512", [&]() {
		//	instanceCount = 16000;
		//} },
		//{ "DownTheValley32kTrue512", [&]() {
		//	instanceCount = 32000;
		//} },
		//{ "DownTheValley64kTrue512", [&]() {
		//	instanceCount = 64000;
		//} },
		//{ "DownTheValley128kTrue512", [&]() {
		//	instanceCount = 128000;
		//} },
		//{ "DownTheValley256kTrue512", [&]() {
		//	instanceCount = 256000;
		//} },
		//{ "DownTheValley512kTrue512", [&]() { TOO SLOW ON 2060
		//	instanceCount = 512000;
		//} }, 
		//{ "DownTheValley1024kTrue512", [&]() { TOO SLOW ON 2060
		//	instanceCount = 1024000;
		//} },
		// Points of that 
		{ "DownTheValley4kMeshShade512", [&]() {
			instanceCount = 4000;
			currentRendererType = RendererType::MESH_SHADED_POINTS;
			loadedPointModels[0]->continousLOD_start = 512.0f;
			loadedPointModels[0]->continousLOD_shallowness = 12.5f;
			loadedPointModels[0]->continousLOD_decay = 1.0f;
		} },
		{ "DownTheValley16kMeshShade512", [&]() {
			instanceCount = 16000;
		} },
		{ "DownTheValley32kMeshShade512", [&]() {
			instanceCount = 32000;
		} },
		{ "DownTheValley64kMeshShade512", [&]() {
			instanceCount = 64000;
		} },
		{ "DownTheValley128kMeshShade512", [&]() {
			instanceCount = 128000;
		} },
		{ "DownTheValley256kMeshShade512", [&]() {
			instanceCount = 256000;
		} },
		//{ "DownTheValley512kMeshShade512", [&]() {
		//	instanceCount = 512000;
		//} },
		//{ "DownTheValley1024kMeshShade512", [&]() {
		//	instanceCount = 1024000;
		//} },
		{ "DownTheValley4kVertexShade512", [&]() {
		instanceCount = 4000;
		currentRendererType = RendererType::VERTEX_SHADED_POINTS;
		loadedPointModels[0]->continousLOD_start = 512.0f;
		loadedPointModels[0]->continousLOD_shallowness = 12.5f;
		loadedPointModels[0]->continousLOD_decay = 1.0f;
		} },
		{ "DownTheValley16kVertexShade512", [&]() {
			instanceCount = 16000;
		} },
		{ "DownTheValley32kVertexShade512", [&]() {
			instanceCount = 32000;
		} },
		{ "DownTheValley64kVertexShade512", [&]() {
			instanceCount = 64000;
		} },
		{ "DownTheValley128kVertexShade512", [&]() {
			instanceCount = 128000;
		} },
		{ "DownTheValley256kVertexShade512", [&]() {
			instanceCount = 256000;
		} }

	};

	bool pauseTimer = false; // If we should wait until capture is complete.

	renderImGui = false; // Turn off imgui

	// For each capture, capture the last captures swap chain. Then run the settings.
	// If running metered mode, capture with NV Perf too
	for (int i = 0; i < rules.size(); i++) {
		if (imageSequenceTimer >= (float)i && imageSequenceTimer < (float)i + 1.0f) {
			if (stageImagesSaved == i - 1) {
				if (i > 0) {
					Graphics::SaveSwapChainImageToFile("./imagesout/" + rules[i - 1].name + ".bmp");
					std::cout << "Completed " << rules[i - 1].name << "\n";
				}
			#ifdef NV_PERF_METER
				Graphics::nvperf_InitiateReport(rules[i].name);
			#endif // NV_PERF_METER
				stageImagesSaved++;
			}
			rules[i].settings();
		#ifdef NV_PERF_METER
			if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\"+ rules[i].name +"\\") pauseTimer = true;
		#endif // NV_PERF_METER
		}
	}
	// All captures complete
	if (imageSequenceTimer > (float)rules.size()) {
		// Grab last capture's image.
		Graphics::SaveSwapChainImageToFile(".\\imagesout\\" + rules[rules.size() - 1].name + ".bmp");
		std::cout << "Completed " << rules[rules.size() - 1].name << "\n";

		captureUnderway = false;
		renderImGui = true;
		
		// If it was a full capture run, finish the application execution here. 
		if (Graphics::IsFullCaptureRunActive()) Input::QuitMainLoop();

		// If metered mode enabled, combine all CSVs into one. 
#ifdef NV_PERF_METER
		// Combine CSVS
		std::vector<std::vector<std::string>> outRows;

		csv2::Reader<csv2::delimiter<','>,
			csv2::quote_character<'"'>,
			csv2::first_row_is_header<true>,
			csv2::trim_policy::trim_whitespace> csv;

		for (auto& rule : rules) {
			if (csv.mmap("nvperfout\\"+ rule.name +"\\nvperf_metrics.csv")) {
				const auto header = csv.header();
				for (const auto& row : csv) {
					outRows.push_back(std::vector<std::string>());
					outRows[outRows.size() - 1].push_back(rule.name);
					for (const auto& cell : row) {
						// Do something with cell value
						std::string value;
						cell.read_value(value);
						outRows[outRows.size() - 1].push_back(value);
					}
				}
			}
		}

		std::ofstream stream("nvperfout\\combined.csv");
		csv2::Writer<csv2::delimiter<','>> writer(stream);

		writer.write_rows(outRows);
		stream.close();

#endif // NV_PERF_METER
	}

	if (!pauseTimer) imageSequenceTimer += Clock::DeltaTime();
	// Say gibberish for fun
	guffer.SayGuff(Clock::DeltaTime());
}
