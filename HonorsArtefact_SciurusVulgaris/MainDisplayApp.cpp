#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>

void MainDisplayApp::Initialize() {

	pointRenderingPass.CreateAll();
	descriptorSizes = { 0, sizeof(WCP_Matrices) * 4000, sizeof(InstancingInfo), sizeof(MeshletInfo), sizeof(LODDataBuffer)};
	descriptorSizesMesh = { sizeof(WCP_Matrices) * 4000, 0};
	//descriptorSizes = { 0, sizeof(WCP_Matrices)};

	myModel = nullptr;
	angle = 0;

	cameraTransform.speed = 0.5f;
	cameraTransform.position = HMM_V3(0, 0, -5);

	pointToRenderCount = 0;
	
	lightShadow_RP.CreateAll();
	sun.SetName("Sun");
	sun.CreateShadowResources(lightShadow_RP.GetRenderPass());

	terrain = new TriListMesh();
	terrain->LoadFile("./models/Terrain004 - Lennart Demes/model.fbx", 0);
	terrain->CopyPointsToVRAM();
	terrainTexture.CreateAndLoadImageFromFile("./models/Terrain004 - Lennart Demes/color.jpg", VK_IMAGE_USAGE_SAMPLED_BIT);
	terrainTexture.CreateImageView();
	size_t sizes[] = { sizeof(WCP_Matrices), 0 };
	terrain->GetDescriptorSet()->Create(pointRenderingPass.GetMeshTraditionalDescriptorSetLayout(), pointRenderingPass.GetMeshTraditionalDescriptorSetLayoutInfo(), sizes);
	terrain->GetDescriptorSet()->UpdateImageSampler(1, &terrainTexture, Graphics::GetBasicLinearSampler());

	treeInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeInstancePositions.ApplyRandomRotation();
	treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));

	pointRenderingPass.GetQuadDescriptorSet()->UpdateImageSampler(4, sun.GetShadowImage(), Graphics::GetBasicNearestSampler());

	instancedMeshTree_RP.CreateAll();
	treeMeshInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeMeshInstancePositions.ApplyRandomRotation();
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)));
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateImageSampler(4, Graphics::GetNoShadowMapImage(), Graphics::GetBasicNearestSampler());

	captureUnderway = false;
	renderImGui = true;
}

void MainDisplayApp::Frame() {
	ImageCaptureSequence();

	if (!meshRenderOn) {
		InstancingInfo instancingInfo;
		LODDataBuffer lodData;

		Graphics::BeginRender();
		lightShadow_RP.BeginRender(&sun);
		if (myModel != nullptr) {

			for (int l = 0; l < myModel->randomLevelsLODPointCount.size(); l++) {
				lodData.maxVertexLevels[l][0] = myModel->randomLevelsLODPointCount[l];
			}
			lodData.maxLevel = myModel->randomLevelsLODPointCount.size();
			lodData.continousDecay = myModel->continousLOD_decay;
			lodData.continousStart = myModel->continousLOD_start;
			lodData.continousShallowness = myModel->continousLOD_shallowness;
			lodData.lodType = static_cast<int>(myModel->levelOfDetailType);
			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeInstancePositions.SetViewAndProjection(sun.GetViewMatrix(HMM_V3(cameraTransform.position.X, 0.0f, cameraTransform.position.Z)), sun.GetProjectionMatrix(100, 50, 50));
			myModel->GetShadowDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
			myModel->GetShadowDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			instancingInfo = { static_cast<uint32_t>(instanceCount), myModel->GetMeshletCount() };




			lightShadow_RP.RenderPointTree(myModel, instancingInfo);


		}
		lightShadow_RP.EndRender();
		// Render logic

		pointRenderingPass.BeginRender(HMM_V4(0, 0, 0, 1));

		cameraTransform.CaptureControls();
		if (myModel != nullptr) {

			//std::vector<WCP_Matrices> dataForUBO(400);

			//for (int x = 0; x < 20; x++) {
			//	for (int y = 0; y < 20; y++) {
			//		dataForUBO[x * 20 + y] = {
			//			//HMM_M4D(1) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Perspective_RH_ZO(70, 1, 0.001, 10)
			//			HMM_Translate(HMM_V3(x * 2.5f, 0, y * 2.5f)) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)),
			//			cameraTransform.viewMatrix,
			//			HMM_Perspective_RH_ZO(70, 1, 0.001, 100)
			//		};
			//	}
			//}

			//LODDataBuffer lodData;
			//for (int l = 0; l < myModel->randomLevelsLODPointCount.size(); l++) {
			//	lodData.maxVertexLevels[l][0] = myModel->randomLevelsLODPointCount[l];
			//}
			//lodData.maxLevel = myModel->randomLevelsLODPointCount.size();
			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
			myModel->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
			myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			//InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};

			//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);
			myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

			//pointRenderingPass.RenderPointTree(myModel, pointToRenderCount);
			pointRenderingPass.RenderPointTreeViaMeshShader(myModel, instancingInfo);
		}




		WCP_Matrices terrainBufferData = {
			HMM_Translate(HMM_V3(-50, 0, 50)),
			cameraTransform.viewMatrix,
			HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100)
		};
		terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
		pointRenderingPass.SwitchToTraditionalMeshPipeline();
		pointRenderingPass.RenderTraditionalMesh(terrain);

		pointRenderingPass.EndRender();

		RenderImGuiControls();

		Light::BufferStruct rawSunData = sun.GetBufferData();
		pointRenderingPass.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);


		pointRenderingPass.ExecuteSecondRender();
		pointRenderingPass.EndSecondRender();
		pointRenderingPass.ExecuteThirdAARender(fxaaEnabled);
		Graphics::FinishImGuiRender();
		pointRenderingPass.EndThirdAARender();
		Graphics::EndRender();
	}
	else {
		InstancingInfo instancingInfo;
		LODDataBuffer lodData;

		Graphics::BeginRender();

		// Render logic
		instancedMeshTree_RP.BeginRender();

		cameraTransform.CaptureControls();
		if (treeMesh.size() > 0) {
			lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

			treeMeshInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
			for (int i = 0; i < treeMesh.size(); i++) {
				treeMesh[i].GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
				//myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

				InstancingInfo instancingInfo{ instanceCount, 0 };

				//myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);
				instancedMeshTree_RP.RenderMeshTree(&treeMesh[i], instancingInfo);
			}
		}




		WCP_Matrices terrainBufferData = {
			HMM_Translate(HMM_V3(-50, 0, 50)),
			cameraTransform.viewMatrix,
			HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100)
		};
		terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
		/*instancedMeshTree_RP.SwitchToTraditionalMeshPipeline();
		instancedMeshTree_RP.RenderTraditionalMesh(terrain);*/

		instancedMeshTree_RP.EndRender();

		RenderImGuiControls();

		Light::BufferStruct rawSunData = sun.GetBufferData();
		instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);


		instancedMeshTree_RP.ExecuteSecondRender();
		Graphics::FinishImGuiRender();
		instancedMeshTree_RP.EndSecondRender();

		Graphics::EndRender();
	}
}

void MainDisplayApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	if (myModel != nullptr) delete myModel;
	delete terrain;
	terrainTexture.Destroy();
	pointRenderingPass.Shutdown();
}

void MainDisplayApp::RenderImGuiControls()
{
	if (!renderImGui) return;
	ImGui::Begin("Point Model Selection");
	ImGui::InputText("Model Path", modelPath, 256);
	if (ImGui::Button("Load")) {
		if (myModel != nullptr) delete myModel;

		myModel = new PointTreeMesh();

		myModel->LoadFromTreeFile(modelPath);
		//myModel->CopyPointsToVRAM();
		descriptorSizes[0] = myModel->GetPointsArraySize(true);
		myModel->CreateDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
		myModel->CreateShadowDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
		myModel->CopyPointsToVRAMMeshBuffer(0);

		MeshletInfo meshletInfo{ myModel->GetMeshletCount() };
		myModel->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
	}
	ImGui::InputText("Mesh Model Path", meshModelPath, 256);
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
	}
	ImGui::SliderAngle("Angle", &angle);
	if (myModel != nullptr) ImGui::SliderInt("N Points", &pointToRenderCount, 0, myModel->points.size());
	if (myModel != nullptr || treeMesh.size() > 0) ImGui::SliderInt("N Instances", &instanceCount, 0, treeInstancePositions.matrices.size());
	if (myModel != nullptr) {
		if (ImGui::Button("Shuffle")) {
			std::random_device rd;
			std::mt19937 g(rd());

			std::shuffle(myModel->points.begin(), myModel->points.end(), g);
			myModel->CopyPointsToVRAM();
		}
	}
	ImGui::Checkbox("FXAA", &fxaaEnabled);
	ImGui::Checkbox("Mesh Render Instead", &meshRenderOn);
	ImGui::Text("FPS %i", Clock::GetFPS());
	ImGui::Text("MS Render %f", Clock::DeltaTime() * 1000);
#ifdef NV_PERF_METER
	if (ImGui::Button("NVPERFRUN")) {
		Graphics::nvperf_InitiateReport();
	}
	ImGui::Text(("Saved to" + Graphics::nfperf_GetLastReportDir()).c_str());
#endif
	if (ImGui::Button("Save Swap Chain")) {
		Graphics::SaveSwapChainImageToFile("./swapchainout.bmp");
	}
	if (ImGui::Button("Run Save Sequence")) {
		imageSequenceTimer = 0;
		captureUnderway = true;
		stageImagesSaved = 0;
	}
	ImGui::End();

	sun.RenderImGuiMenu(true);
}

void MainDisplayApp::ImageCaptureSequence()
{
	if (!captureUnderway) return;

	imageSequenceTimer += Clock::DeltaTime();

	renderImGui = false;
	if (imageSequenceTimer < 1.0f) {
		cameraTransform.position = HMM_V3(0, 30, 0);
		cameraTransform.euler = HMM_V3(-45, 0, 0);
		instanceCount = 4000;
	}
	else if (imageSequenceTimer < 2.0f) {
		if (stageImagesSaved == 0) {
			Graphics::SaveSwapChainImageToFile("./Render001.bmp");
			stageImagesSaved++;
		}
		cameraTransform.position = HMM_V3(0, 30, 0);
		cameraTransform.euler = HMM_V3(-45, -90, 0);
		instanceCount = 1000;
	}
	else if (imageSequenceTimer < 3.0f) {
		if (stageImagesSaved == 1) {
			Graphics::SaveSwapChainImageToFile("./Render002.bmp");
			stageImagesSaved++;
		}
		cameraTransform.position = HMM_V3(0, 30, 0);
		cameraTransform.euler = HMM_V3(-45, 90, 0);
		instanceCount = 100;
	}
	else {
		if (stageImagesSaved == 2) {
			Graphics::SaveSwapChainImageToFile("./Render003.bmp");
			stageImagesSaved++;
		}
		captureUnderway = false;
		renderImGui = true;
	}
}
