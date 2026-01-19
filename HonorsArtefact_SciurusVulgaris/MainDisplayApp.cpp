#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>

void MainDisplayApp::Initialize() {

	pointRenderingPass.CreateAll();
	descriptorSizes = { sizeof(WCP_Matrices) * 400, sizeof(LODDataBuffer)};

	myModel = nullptr;
	angle = 0;

	cameraTransform.speed = 0.5f;
	cameraTransform.position = HMM_V3(0, 0, -5);

	pointToRenderCount = 0;
}

void MainDisplayApp::Frame() {
	

	// Render logic
	Graphics::BeginRender();
	pointRenderingPass.BeginRender(HMM_V4(0, 0, 0, 1));

	if (myModel != nullptr) {
		cameraTransform.CaptureControls();
		std::vector<WCP_Matrices> dataForUBO(400);

		for (int x = 0; x < 20; x++) {
			for (int y = 0; y < 20; y++) {
				dataForUBO[x * 20 + y] = {
					//HMM_M4D(1) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Perspective_RH_ZO(70, 1, 0.001, 10)
					HMM_Translate(HMM_V3(x * 2.5f, 0, y * 2.5f)) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)),
					cameraTransform.viewMatrix,
					HMM_Perspective_RH_ZO(70, 1, 0.001, 100)
				};
			}
		}

		LODDataBuffer lodData;
		for (int l = 0; l < myModel->randomLevelsLODPointCount.size(); l++) {
			lodData.maxVertexLevels[l][0] = myModel->randomLevelsLODPointCount[l];
		}
		lodData.maxLevel = myModel->randomLevelsLODPointCount.size();
		lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

		myModel->GetDescriptorSet()->UpdateStorageBufferData(0, dataForUBO.data());
		myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &lodData);
		pointRenderingPass.Render(myModel, pointToRenderCount);
	}

	ImGui::Begin("Point Model Selection");
	ImGui::InputText("Model Path", modelPath, 256);
	if (ImGui::Button("Load")) {
		if (myModel != nullptr) delete myModel;

		myModel = new PointTreeMesh();

		myModel->LoadFromTreeFile(modelPath);
		myModel->CopyPointsToVRAM();
		myModel->CreateDescriptorSet(pointRenderingPass.GetDescriptorSetLayout(), pointRenderingPass.GetDescriptorSetLayoutInfo(), descriptorSizes.data());
	}
	ImGui::SliderAngle("Angle", &angle);
	if (myModel != nullptr) ImGui::SliderInt("N Points", &pointToRenderCount, 0, myModel->points.size());
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

	pointRenderingPass.EndRender();

	pointRenderingPass.ExecuteSecondRender();
	Graphics::FinishImGuiRender();
	pointRenderingPass.EndSecondRender();
	Graphics::EndRender();
}

void MainDisplayApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	if (myModel != nullptr) delete myModel;
	pointRenderingPass.Shutdown();
}