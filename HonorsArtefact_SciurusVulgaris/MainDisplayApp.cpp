#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>

void MainDisplayApp::Initialize() {

	pointRenderingPass.CreateAll();
	descriptorSizes = { 0, sizeof(WCP_Matrices) * 4000, sizeof(InstancingInfo), sizeof(MeshletInfo)};
	//descriptorSizes = { 0, sizeof(WCP_Matrices)};

	myModel = nullptr;
	angle = 0;

	cameraTransform.speed = 0.5f;
	cameraTransform.position = HMM_V3(0, 0, -5);

	pointToRenderCount = 0;

	sun.SetName("Sun");

	terrain = new TriListMesh();
	terrain->LoadFile("./models/Terrain004 - Lennart Demes/model.fbx", 0);
	terrain->CopyPointsToVRAM();
	terrainTexture.CreateAndLoadImageFromFile("./models/Terrain004 - Lennart Demes/color.jpg", VK_IMAGE_USAGE_SAMPLED_BIT);
	terrainTexture.CreateImageView();
	size_t sizes[] = { sizeof(WCP_Matrices), 0 };
	terrain->GetDescriptorSet()->Create(pointRenderingPass.GetMeshTraditionalDescriptorSetLayout(), pointRenderingPass.GetMeshTraditionalDescriptorSetLayoutInfo(), sizes);
	terrain->GetDescriptorSet()->UpdateImageSampler(1, &terrainTexture, Graphics::GetBasicLinearSampler());

	treeInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 2, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));
}

void MainDisplayApp::Frame() {
	

	// Render logic
	Graphics::BeginRender();
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

		LODDataBuffer lodData;
		for (int l = 0; l < myModel->randomLevelsLODPointCount.size(); l++) {
			lodData.maxVertexLevels[l][0] = myModel->randomLevelsLODPointCount[l];
		}
		lodData.maxLevel = myModel->randomLevelsLODPointCount.size();
		lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

		treeInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, 1, 0.001, 100));
		myModel->GetDescriptorSet()->UpdateStorageBufferData(1, treeInstancePositions.matrices.data());
		//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &lodData);

		InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};
		
		//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);
		myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

		//pointRenderingPass.RenderPointTree(myModel, pointToRenderCount);
		pointRenderingPass.RenderPointTreeViaMeshShader(myModel, instancingInfo);
	}

	ImGui::Begin("Point Model Selection");
	ImGui::InputText("Model Path", modelPath, 256);
	if (ImGui::Button("Load")) {
		if (myModel != nullptr) delete myModel;

		myModel = new PointTreeMesh();

		myModel->LoadFromTreeFile(modelPath);
		//myModel->CopyPointsToVRAM();
		descriptorSizes[0] = myModel->GetPointsArraySize(true);
		myModel->CreateDescriptorSet(pointRenderingPass.GetMeshShadeDescriptorSetLayout(), pointRenderingPass.GetMeshShadeDescriptorSetLayoutInfo(), descriptorSizes.data());
		myModel->CopyPointsToVRAMMeshBuffer(0);

		MeshletInfo meshletInfo{myModel->GetMeshletCount()};
		myModel->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
	}
	ImGui::SliderAngle("Angle", &angle);
	if (myModel != nullptr) ImGui::SliderInt("N Points", &pointToRenderCount, 0, myModel->points.size());
	if (myModel != nullptr) ImGui::SliderInt("N Instances", &instanceCount, 0, treeInstancePositions.matrices.size());
	if (myModel != nullptr) {
		if (ImGui::Button("Shuffle")) {
			std::random_device rd;
			std::mt19937 g(rd());

			std::shuffle(myModel->points.begin(), myModel->points.end(), g);
			myModel->CopyPointsToVRAM();
		}
	}

	ImGui::Text("FPS %i", Clock::GetFPS());
	ImGui::End();


	WCP_Matrices terrainBufferData = {
		HMM_Translate(HMM_V3(-50, 0, 50)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(70, 1, 0.001, 100)
	};
	terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
	pointRenderingPass.SwitchToTraditionalMeshPipeline();
	pointRenderingPass.RenderTraditionalMesh(terrain);

	pointRenderingPass.EndRender();

	sun.RenderImGuiMenu(true);
	Light::BufferStruct rawSunData = sun.GetBufferData();
	pointRenderingPass.GetQuadDescriptorSet()->UpdateUniformBufferData(3, &rawSunData);

	pointRenderingPass.ExecuteSecondRender();
	Graphics::FinishImGuiRender();
	pointRenderingPass.EndSecondRender();
	Graphics::EndRender();
}

void MainDisplayApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	if (myModel != nullptr) delete myModel;
	delete terrain;
	terrainTexture.Destroy();
	pointRenderingPass.Shutdown();
}