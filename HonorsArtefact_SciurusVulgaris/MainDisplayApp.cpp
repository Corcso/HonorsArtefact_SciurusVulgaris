#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>
#include "csv2.hpp"

void MainDisplayApp::Initialize() {

	pointRenderingPass.CreateAll();
	descriptorSizes = { 0, sizeof(HMM_Mat4) * MAX_INSTANCE_POSITIONS, sizeof(InstancingInfo), sizeof(MeshletInfo), sizeof(LODDataBuffer), sizeof(TAAInfo), sizeof(VP_Matrices) * 2};
	descriptorSizesMesh = { sizeof(HMM_Mat4) * MAX_INSTANCE_POSITIONS, 0, sizeof(VP_Matrices) * 2 };
	
	angle = 0;

	cameraTransform.speed = 0.5f;
	cameraTransform.position = HMM_V3(0, 30, 0);
	cameraTransform.euler = HMM_V3(-45, 0, 0);

	pointToRenderCount = 0;
	
	lightShadow_RP.CreateAll();
	sun.SetName("Sun");
	sun.CreateShadowResources(lightShadow_RP.GetRenderPass());

	terrain = new TriListMesh();
	//terrain->LoadFile("./models/Terrain004 - Lennart Demes/model.fbx", 0);
	terrain->LoadFile("./models/ChinaValley/ChinaValley.fbx", 0);
	terrain->CopyPointsToVRAM();
	// ChinaValley map from (Mustoe-Playfair, 2026)
	terrainTexture.CreateAndLoadImageFromFile("./models/ChinaValley/Colour.png", VK_IMAGE_USAGE_SAMPLED_BIT);
	terrainTexture.CreateImageView();
	size_t sizes[] = { sizeof(WCP_Matrices) * 2, 0, sizeof(TAAInfo)};
	terrain->GetDescriptorSet()->Create(pointRenderingPass.GetMeshTraditionalDescriptorSetLayout(), pointRenderingPass.GetMeshTraditionalDescriptorSetLayoutInfo(), sizes);
	terrain->GetDescriptorSet()->UpdateImageSampler(1, &terrainTexture, Graphics::GetBasicLinearSampler());

	//treeInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeInstancePositions.LoadFromFile("./models/ChinaValley/ChinaValleyLocations1024K.obj");
	uint64_t chosenSeed = treeInstancePositions.ApplyRandomRotation(Graphics::IsFullCaptureRunActive() ? 1 : 0);
	//treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));
	treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 0, 0)) * HMM_Rotate_LH(3.141 / 2000.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)));
	treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.03, 0.03, 0.03)));

	pointRenderingPass.GetQuadDescriptorSet()->UpdateImageSampler(4, sun.GetShadowImage(), Graphics::GetBasicNearestSampler());

	instancedMeshTree_RP.CreateAll();
	treeMeshInstancePositions.LoadFromFile("./models/ChinaValley/ChinaValleyLocations1024K.obj");
	treeMeshInstancePositions.ApplyRandomRotation(chosenSeed);
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 0, 0)) * HMM_Rotate_LH(3.141 / 2000.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)));
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.03, 0.03, 0.03)));
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateImageSampler(4, Graphics::GetNoShadowMapImage(), Graphics::GetBasicNearestSampler());

	captureUnderway = false;
	renderImGui = true;

	currentRendererType = RendererType::MESH_SHADED_POINTS;

	// If we are doing a full capture run, load the relevant models
	if (Graphics::IsFullCaptureRunActive()) {
		// Use default paths
		// Load Point Mesh
		std::cout << "Loading Point Tree (Summer Bubble)...\n";
		{
			for (auto& model : loadedPointModels) {
				if (model != nullptr) {
					Graphics::WaitUntilGPUIdle(); // TODO fix this or not allow it. 
					delete model;
					model = nullptr;
				}
			}

			loadedPointModels.clear();

			loadedPointModels.push_back(new PointTreeMesh());

			loadedPointModels[0]->LoadFromTreeFile(modelPath);
			//myModel->CopyPointsToVRAM();
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
	ImageCaptureSequence();

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

	if (sun.IsShadowEnabled()) {
		Graphics::PushMetricRange("Shadow Map Render");
		lightShadow_RP.BeginRender(&sun);
		uint32_t index = 0;
		for (auto& model : loadedPointModels) {
			if (model != nullptr) {
				lodData.cameraPosition = HMM_V4(cameraTransform.position.X, 0, cameraTransform.position.Z, 1);

				VP_Matrices shadowMap = { sun.GetViewMatrix(HMM_V3(cameraTransform.position.X, 0.0f, cameraTransform.position.Z)), sun.GetProjectionMatrix(150, 150, 150) };
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(6, &shadowMap);
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(4, &lodData);

				//myModel->GetShadowDescriptorSet()->FlushBuffer(1);
				instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount(), static_cast<uint32_t>(loadedPointModels.size()), index };
				model->GetShadowDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

				lightShadow_RP.RenderPointTree(model, instancingInfo);
			}
			index++;
		}
		lightShadow_RP.EndRender();
		Graphics::PopMetricRange();
	}
	// Render logic
	Graphics::PushMetricRange("Render Point Trees");

	pointRenderingPass.UpdateCameraInfoForSkybox(cameraTransform);
	pointRenderingPass.BeginRenderMeshShade(HMM_V4(0, 0, 0, 1));

	uint32_t index = 0;
	for (auto& model : loadedPointModels) {
		if (model != nullptr) {

			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeInstanceViewProjThisAndLastFrame[0] = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };
			model->GetDescriptorSet()->UpdateUniformBufferData(6, &treeInstanceViewProjThisAndLastFrame);
			model->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			//InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};

			//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);

			instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount(), static_cast<uint32_t>(loadedPointModels.size()), index };
			model->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

			pointRenderingPass.UpdateTAADescriptor(model->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

			//pointRenderingPass.RenderPointTree(myModel, pointToRenderCount);
			pointRenderingPass.RenderPointTreeViaMeshShader(model, instancingInfo);
		}
		index++;
	}




	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(0, 0, 0)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000)
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

void MainDisplayApp::FrameVertexShaded()
{
	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	Graphics::BeginRender();

	// Render logic
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

			treeInstanceViewProjThisAndLastFrame[0] = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };
			model->GetDescriptorSet()->UpdateUniformBufferData(6, &treeInstanceViewProjThisAndLastFrame);
			model->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			//InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};

			//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);
			instancingInfo = { static_cast<uint32_t>(instanceCount / loadedPointModels.size()), model->GetMeshletCount() , static_cast<uint32_t>(loadedPointModels.size()), index };
			model->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

			pointRenderingPass.UpdateTAADescriptor(model->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

			pointRenderingPass.RenderPointTree(model, instancingInfo);
			//pointRenderingPass.RenderPointTreeViaMeshShader(myModel, instancingInfo);
		}
		index++;
	}




	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(0, 0, 0)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000)
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

		//treeMeshInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000));
		for (int i = 0; i < triangleMeshTreeLoader.GetMeshVector()->size(); i++) {
			//(*triangleMeshTreeLoader.GetMeshVector())[i].GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
			//myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			InstancingInfo instancingInfo{ instanceCount, 0 };
			VP_Matrices viewCamMatrices = { cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.1, 1000) };

			(*triangleMeshTreeLoader.GetMeshVector())[i].GetDescriptorSet()->UpdateUniformBufferData(2, &viewCamMatrices); // Only 1/2 of buffer update but second half not used.
			instancedMeshTree_RP.RenderMeshTree(&(*triangleMeshTreeLoader.GetMeshVector())[i], instancingInfo);
		}
	}
	Graphics::PopMetricRange();

	instancedMeshTree_RP.EndRender();

	RenderImGuiControls();

	Light::BufferStruct rawSunData = sun.GetBufferData();
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);

	Graphics::PushMetricRange("Colour Deferred");
	instancedMeshTree_RP.ExecuteSecondRender();
	Graphics::FinishImGuiRender();
	instancedMeshTree_RP.EndSecondRender();

	Graphics::EndRender();
}

void MainDisplayApp::RenderImGuiControls()
{
	if (!renderImGui) return;
	ImGui::Begin("Point Model Selection");
	ImGui::InputText("Model Path", modelPath, 256);
	if (ImGui::Button("Load Another")) {
		loadedPointModels.push_back(new PointTreeMesh());

		loadedPointModels[loadedPointModels.size() - 1]->LoadFromTreeFile(modelPath);
		//myModel->CopyPointsToVRAM();
		descriptorSizes[0] = loadedPointModels[loadedPointModels.size() - 1]->GetPointsArraySize(true);
		loadedPointModels[loadedPointModels.size() - 1]->CreateDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
		loadedPointModels[loadedPointModels.size() - 1]->CreateShadowDescriptorSet(lightShadow_RP.GetShadowSetLayout(), lightShadow_RP.GetShadowSetLayoutInfo(), descriptorSizes.data());
		loadedPointModels[loadedPointModels.size() - 1]->CopyPointsToVRAMMeshBuffer(0);
		loadedPointModels[loadedPointModels.size() - 1]->CopyPointsToVRAM();

		// Copy instance positions
		loadedPointModels[loadedPointModels.size() - 1]->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
		loadedPointModels[loadedPointModels.size() - 1]->GetShadowDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
		

		MeshletInfo meshletInfo{ loadedPointModels[loadedPointModels.size() - 1]->GetMeshletCount() };
		loadedPointModels[loadedPointModels.size() - 1]->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
	}
	/*ImGui::InputText("Mesh Model Path", meshModelPath, 256);
	if (ImGui::Button("Load Mesh")) {
		if (treeMesh.size() > 0) treeMesh.clear();

		treeMesh = TriListMesh::LoadMultiMeshFile(meshModelPath);

		treeMeshTextures.resize(2);
		treeMeshTextures[0].CreateAndLoadImageFromFile("./models/SpeedTrees/singleAColor.png", VK_IMAGE_USAGE_SAMPLED_BIT);
		treeMeshTextures[0].CreateImageView();
		treeMeshTextures[1].CreateAndLoadImageFromFile("./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", VK_IMAGE_USAGE_SAMPLED_BIT);
		treeMeshTextures[1].CreateImageView();

		for (int i = 0; i < treeMesh.size(); i++) {
			treeMesh[i].CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
			treeMesh[i].GetDescriptorSet()->UpdateImageSampler(1, &treeMeshTextures[i], Graphics::GetBasicLinearSampler());
		}
	}*/
	

	

	//ImGui::SliderAngle("Angle", &angle);
	//if (myModel != nullptr) ImGui::SliderInt("N Points", &pointToRenderCount, 0, myModel->points.size());
	//
	//if (myModel != nullptr) {
	//	if (ImGui::Button("Shuffle")) {
	//		std::random_device rd;
	//		std::mt19937 g(rd());

	//		std::shuffle(myModel->points.begin(), myModel->points.end(), g);
	//		myModel->CopyPointsToVRAM();
	//	}
	//}
	
	//ImGui::Checkbox("Mesh Render Instead", &meshRenderOn);
	ImGui::End();

	ImGui::Begin("Meterage");
	ImGui::Text("FPS %i", Clock::GetFPS());
	ImGui::Text("MS Render %f", Clock::DeltaTime() * 1000);
#ifdef NV_PERF_METER
	if (ImGui::Button("NVPERFRUN")) {
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

	ImGui::Begin("Triangle Model");
	triangleMeshTreeLoader.Display([&](TriListMesh* mesh, Image* texture) {
		mesh->CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
		mesh->GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
		mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, Graphics::GetBasicLinearSampler());
		});
	ImGui::End();

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

	ImGui::Begin("Live LOD Edits");

	if ((loadedPointModels.size() > 0 && loadedPointModels[0] != nullptr) && loadedPointModels[0]->levelOfDetailType == PointTreeMesh::LODType::CONTINUOUS) {
		ImGui::DragFloat("Level 0 Points", &loadedPointModels[0]->continousLOD_start, 128, 0, loadedPointModels[0]->points.size());
		ImGui::DragFloat("Shallowness", &loadedPointModels[0]->continousLOD_shallowness, 1, 0, 100);
		ImGui::DragFloat("Decay", &loadedPointModels[0]->continousLOD_decay, 0.1f, 1, 5);
	}

	ImGui::End();

	ImGui::Begin("Shadow Map");
	ImGui::Image(sun.GetShadowImageImGuiTex(), ImVec2(800, 800));
	ImGui::End();

	ImGui::Begin("Camera Position Information");
	ImGui::Text("Pos: %f %f %f", cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z);
	ImGui::Text("Rot: %f %f %f", cameraTransform.euler.X, cameraTransform.euler.Y, cameraTransform.euler.Z);
	ImGui::End();

	ImGui::Begin("Lighting");
	ImGui::Checkbox("Enable Skybox", &pointRenderingPass.enableSkybox);
	sun.RenderImGuiMenu(false);
	ImGui::End();
}

void MainDisplayApp::ImageCaptureSequence()
{
	if (!captureUnderway) return;

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

	//std::cout << Graphics::nfperf_GetLastReportDir() << "\n";
	bool pauseTimer = false;

	renderImGui = false;
//	if (imageSequenceTimer < 1.0f) {
//		if (stageImagesSaved == -1) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001a");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 4000;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001a\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 2.0f) {
//		if (stageImagesSaved == 0) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001b");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 3000;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001b\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 3.0f) {
//		if (stageImagesSaved == 1) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001c");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 2000;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001c\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 4.0f) {
//		if (stageImagesSaved == 2) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001d");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 1000;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001d\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 5.0f) {
//		if (stageImagesSaved == 3) {
//			Graphics::SaveSwapChainImageToFile("./Render001e.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001e");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 750;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001e\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 6.0f) {
//		if (stageImagesSaved == 4) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001f");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 500;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001f\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 7.0f) {
//		if (stageImagesSaved == 5) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001g");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 250;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001g\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//	else if (imageSequenceTimer < 8.0f) {
//		if (stageImagesSaved == 6) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render001h");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 0, 0);
//		instanceCount = 125;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render001h\\") pauseTimer = true;
//#endif // NV_PERF_METER
//		}
//
//	else if (imageSequenceTimer < 9.0f) {
//		if (stageImagesSaved == 7) {
//			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render002");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, -90, 0);
//		instanceCount = 1000;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render002\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//
//	else if (imageSequenceTimer < 10.0f) {
//		if (stageImagesSaved == 8) {
//			Graphics::SaveSwapChainImageToFile("./Render002.bmp");
//#ifdef NV_PERF_METER
//			Graphics::nvperf_InitiateReport("Render003");
//#endif // NV_PERF_METER
//			stageImagesSaved++;
//		}
//		cameraTransform.position = HMM_V3(0, 30, 0);
//		cameraTransform.euler = HMM_V3(-45, 90, 0);
//		instanceCount = 100;
//#ifdef NV_PERF_METER
//		if (Graphics::nfperf_GetLastReportDir() != "nvperfout\\Render003\\") pauseTimer = true;
//#endif // NV_PERF_METER
//	}
//
//	else {
//		if (stageImagesSaved == 9) {
//			Graphics::SaveSwapChainImageToFile("./Render003.bmp");
//			stageImagesSaved++;
//		}
//		captureUnderway = false;
//		renderImGui = true;
//
//#ifdef NV_PERF_METER
//		// Combine CSVS
//		std::vector<std::pair<std::string, std::string>> reportsToCombine{
//			{"nvperfout\\Render001a\\nvperf_metrics_summary.csv", "1a"},
//			{"nvperfout\\Render001b\\nvperf_metrics_summary.csv", "1b"},
//			{"nvperfout\\Render001c\\nvperf_metrics_summary.csv", "1c"},
//			{"nvperfout\\Render001d\\nvperf_metrics_summary.csv", "1d"},
//			{"nvperfout\\Render001e\\nvperf_metrics_summary.csv", "1e"},
//			{"nvperfout\\Render001f\\nvperf_metrics_summary.csv", "1f"},
//			{"nvperfout\\Render001g\\nvperf_metrics_summary.csv", "1g"},
//			{"nvperfout\\Render001h\\nvperf_metrics_summary.csv", "1h"},
//			{"nvperfout\\Render002\\nvperf_metrics_summary.csv", "2"},
//			{"nvperfout\\Render003\\nvperf_metrics_summary.csv", "3"},
//		};
//		std::vector<std::vector<std::string>> outRows;
//
//		csv2::Reader<csv2::delimiter<','>,
//			csv2::quote_character<'"'>,
//			csv2::first_row_is_header<true>,
//			csv2::trim_policy::trim_whitespace> csv;
//
//		for (auto& report : reportsToCombine) {
//			if (csv.mmap(report.first)) {
//				const auto header = csv.header();
//				for (const auto& row : csv) {
//					outRows.push_back(std::vector<std::string>());
//					outRows[outRows.size() - 1].push_back(report.second);
//					for (const auto& cell : row) {
//						// Do something with cell value
//						std::string value;
//						cell.read_value(value);
//						outRows[outRows.size() - 1].push_back(value);
//					}
//				}
//			}
//		}
//
//		std::ofstream stream("nvperfout\\combined.csv");
//		csv2::Writer<csv2::delimiter<','>> writer(stream);
//
//		writer.write_rows(outRows);
//		stream.close();
//
//#endif // NV_PERF_METER
//	}


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
	if (imageSequenceTimer > (float)rules.size()) {
		Graphics::SaveSwapChainImageToFile(".\\imagesout\\" + rules[rules.size() - 1].name + ".bmp");
		std::cout << "Completed " << rules[rules.size() - 1].name << "\n";
		//Graphics::SaveSwapChainImageToFile("./Render003.bmp");
		//stageImagesSaved++;
		captureUnderway = false;
		renderImGui = true;

		if (Graphics::IsFullCaptureRunActive()) Input::QuitMainLoop();

#ifdef NV_PERF_METER
		// Combine CSVS
		/*std::vector<std::pair<std::string, std::string>> reportsToCombine{
			{"nvperfout\\Render001a\\nvperf_metrics_summary.csv", "1a"},
			{"nvperfout\\Render001b\\nvperf_metrics_summary.csv", "1b"},
			{"nvperfout\\Render001c\\nvperf_metrics_summary.csv", "1c"},
			{"nvperfout\\Render001d\\nvperf_metrics_summary.csv", "1d"},
			{"nvperfout\\Render001e\\nvperf_metrics_summary.csv", "1e"},
			{"nvperfout\\Render001f\\nvperf_metrics_summary.csv", "1f"},
			{"nvperfout\\Render001g\\nvperf_metrics_summary.csv", "1g"},
			{"nvperfout\\Render001h\\nvperf_metrics_summary.csv", "1h"},
			{"nvperfout\\Render002\\nvperf_metrics_summary.csv", "2"},
			{"nvperfout\\Render003\\nvperf_metrics_summary.csv", "3"},
		};*/
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
	guffer.SayGuff(Clock::DeltaTime());
}
