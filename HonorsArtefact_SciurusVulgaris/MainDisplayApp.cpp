#include "PCH.h"
#include "MainDisplayApp.h"
#include "Clock.h"
#include "Input.h"
#include "Graphics.h"
#include <random>
#include "csv2.hpp"

void MainDisplayApp::Initialize() {

	pointRenderingPass.CreateAll();
	descriptorSizes = { 0, sizeof(WCP_Matrices) * 8000, sizeof(InstancingInfo), sizeof(MeshletInfo), sizeof(LODDataBuffer), sizeof(TAAInfo)};
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
	size_t sizes[] = { sizeof(WCP_Matrices) * 2, 0, sizeof(TAAInfo)};
	terrain->GetDescriptorSet()->Create(pointRenderingPass.GetMeshTraditionalDescriptorSetLayout(), pointRenderingPass.GetMeshTraditionalDescriptorSetLayoutInfo(), sizes);
	terrain->GetDescriptorSet()->UpdateImageSampler(1, &terrainTexture, Graphics::GetBasicLinearSampler());

	treeInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeInstancePositions.ApplyRandomRotation();
	treeInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));
	treeInstancePositionsLastFrame.matrices.resize(4000);// = treeInstancePositions.matrices;

	pointRenderingPass.GetQuadDescriptorSet()->UpdateImageSampler(4, sun.GetShadowImage(), Graphics::GetBasicNearestSampler());

	instancedMeshTree_RP.CreateAll();
	treeMeshInstancePositions.LoadFromFile("./models/Terrain004 - Lennart Demes/InstanceData4k.obj");
	treeMeshInstancePositions.ApplyRandomRotation();
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)));
	treeMeshInstancePositions.ApplyAlternateTransform(HMM_Translate(HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(0.1, 0.1, 0.1)));
	instancedMeshTree_RP.GetQuadDescriptorSet()->UpdateImageSampler(4, Graphics::GetNoShadowMapImage(), Graphics::GetBasicNearestSampler());

	captureUnderway = false;
	renderImGui = true;

	currentRendererType = RendererType::MESH_SHADED_POINTS;
}

void MainDisplayApp::Frame() {
	ImageCaptureSequence();

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
	if (myModel != nullptr) delete myModel;
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

	cameraTransform.CaptureControls();

	Graphics::BeginRender();
	Graphics::PushMetricRange("Shadow Map Render");
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
		std::vector<WCP_Matrices> copiedTemp(8000);
		memcpy(copiedTemp.data(), treeInstancePositions.matrices.data(), sizeof(WCP_Matrices) * 4000);
		memcpy(copiedTemp.data() + 4000, treeInstancePositions.matrices.data(), sizeof(WCP_Matrices) * 4000);
		//ZeroMemory(copiedTemp.data() + 4000, sizeof(WCP_Matrices) * 4000);
		myModel->GetShadowDescriptorSet()->UpdateStorageBufferData(1, copiedTemp.data());
		myModel->GetShadowDescriptorSet()->UpdateUniformBufferData(4, &lodData);

		instancingInfo = { static_cast<uint32_t>(instanceCount), myModel->GetMeshletCount() };

		myModel->GetShadowDescriptorSet()->FlushBuffer(1);


		lightShadow_RP.RenderPointTree(myModel, instancingInfo);


	}
	lightShadow_RP.EndRender();
	Graphics::PopMetricRange();
	// Render logic
	Graphics::PushMetricRange("Render Point Trees");
	pointRenderingPass.BeginRenderMeshShade(HMM_V4(0, 0, 0, 1));


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
		std::vector<WCP_Matrices> copiedTemp(8000);
		memcpy(copiedTemp.data(), treeInstancePositions.matrices.data(), sizeof(WCP_Matrices) * 4000);
		memcpy(copiedTemp.data() + 4000, treeInstancePositionsLastFrame.matrices.data(), sizeof(WCP_Matrices) * 4000);
		myModel->GetDescriptorSet()->UpdateStorageBufferData(1, copiedTemp.data());
		myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

		//InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};

		//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);
		myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

		pointRenderingPass.UpdateTAADescriptor(myModel->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

		//pointRenderingPass.RenderPointTree(myModel, pointToRenderCount);
		pointRenderingPass.RenderPointTreeViaMeshShader(myModel, instancingInfo);
	}




	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(-50, 0, 50)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100)
		},
		{
		HMM_Translate(HMM_V3(-50, 0, 50)),
		treeInstancePositionsLastFrame.matrices[0].camera,
		treeInstancePositionsLastFrame.matrices[0].projection
		}
	};
	terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
	pointRenderingPass.UpdateTAADescriptor(terrain->GetDescriptorSet(), 2, taaEnabled, taaLogarithmicColorSpace);
	Graphics::PopMetricRange();
	pointRenderingPass.SwitchToTraditionalMeshPipeline();
	Graphics::PushMetricRange("Terrain Mesh Render");
	pointRenderingPass.RenderTraditionalMesh(terrain);

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

	treeInstancePositionsLastFrame.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
}

void MainDisplayApp::FrameVertexShaded()
{
	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	cameraTransform.CaptureControls();

	Graphics::BeginRender();

	// Render logic
	Graphics::PushMetricRange("Render Point Trees");
	pointRenderingPass.BeginRenderVertexShade(HMM_V4(0, 0, 0, 1));

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

		treeInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
		std::vector<WCP_Matrices> copiedTemp(8000);
		memcpy(copiedTemp.data(), treeInstancePositions.matrices.data(), sizeof(WCP_Matrices) * 4000);
		memcpy(copiedTemp.data() + 4000, treeInstancePositionsLastFrame.matrices.data(), sizeof(WCP_Matrices) * 4000);
		myModel->GetDescriptorSet()->UpdateStorageBufferData(1, copiedTemp.data());
		myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

		//InstancingInfo instancingInfo{ instanceCount, myModel->GetMeshletCount()};

		//myModel->GetDescriptorSet()->UpdateUniformBufferData(1, &treeInstancePositions.matrices[0]);
		instancingInfo = { static_cast<uint32_t>(instanceCount), myModel->GetMeshletCount() };
		myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);

		pointRenderingPass.UpdateTAADescriptor(myModel->GetDescriptorSet(), 5, taaEnabled, taaLogarithmicColorSpace);

		pointRenderingPass.RenderPointTree(myModel, instancingInfo);
		//pointRenderingPass.RenderPointTreeViaMeshShader(myModel, instancingInfo);
	}




	WCP_Matrices terrainBufferData[2] = { {
		HMM_Translate(HMM_V3(-50, 0, 50)),
		cameraTransform.viewMatrix,
		HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100)
		},
		{
		HMM_Translate(HMM_V3(-50, 0, 50)),
		treeInstancePositionsLastFrame.matrices[0].camera,
		treeInstancePositionsLastFrame.matrices[0].projection
		}
	};
	terrain->GetDescriptorSet()->UpdateUniformBufferData(0, &terrainBufferData);
	pointRenderingPass.UpdateTAADescriptor(terrain->GetDescriptorSet(), 2, taaEnabled, taaLogarithmicColorSpace);
	Graphics::PopMetricRange();
	pointRenderingPass.SwitchToTraditionalMeshPipeline();
	Graphics::PushMetricRange("Terrain Mesh Render");
	pointRenderingPass.RenderTraditionalMesh(terrain);

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

	treeInstancePositionsLastFrame.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
}

void MainDisplayApp::FrameMeshTrue()
{
	InstancingInfo instancingInfo;
	LODDataBuffer lodData;

	Graphics::BeginRender();

	// Render logic
	instancedMeshTree_RP.BeginRender();

	cameraTransform.CaptureControls();
	if (triangleMeshTreeLoader.GetMeshVector()->size() > 0) {
		lodData.cameraPosition = HMM_V4(cameraTransform.position.X, cameraTransform.position.Y, cameraTransform.position.Z, 1);

		treeMeshInstancePositions.SetViewAndProjection(cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, Graphics::GetSwapChainExtent().width / (float)Graphics::GetSwapChainExtent().height, 0.001, 100));
		for (int i = 0; i < triangleMeshTreeLoader.GetMeshVector()->size(); i++) {
			(*triangleMeshTreeLoader.GetMeshVector())[i].GetDescriptorSet()->UpdateStorageBufferData(0, treeMeshInstancePositions.matrices.data());
			//myModel->GetDescriptorSet()->UpdateUniformBufferData(4, &lodData);

			InstancingInfo instancingInfo{ instanceCount, 0 };

			//myModel->GetDescriptorSet()->UpdateUniformBufferData(2, &instancingInfo);
			instancedMeshTree_RP.RenderMeshTree(&(*triangleMeshTreeLoader.GetMeshVector())[i], instancingInfo);
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
		myModel->CreateShadowDescriptorSet(lightShadow_RP.GetShadowSetLayout(), lightShadow_RP.GetShadowSetLayoutInfo(), descriptorSizes.data());
		myModel->CopyPointsToVRAMMeshBuffer(0);
		myModel->CopyPointsToVRAM();

		MeshletInfo meshletInfo{ myModel->GetMeshletCount() };
		myModel->GetDescriptorSet()->UpdateUniformBufferData(3, &meshletInfo);
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
	ImGui::PushID("TriangleMeshTreeLoader");
	triangleMeshTreeLoader.Display([&](TriListMesh* mesh, Image* texture) {
			mesh->CreateDescriptorSet(instancedMeshTree_RP.GetDescriptorSetLayout(), instancedMeshTree_RP.GetDescriptorSetLayoutInfo(), descriptorSizesMesh.data());
			mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, Graphics::GetBasicLinearSampler());
		});
	ImGui::PopID();

	const char* items[] = { "True Mesh", "Mesh Shaded Points", "Vertex Shaded Points"};
	ImGui::Combo("Renderer", reinterpret_cast<int*>(&currentRendererType), items, 3);

	ImGui::SliderAngle("Angle", &angle);
	if (myModel != nullptr) ImGui::SliderInt("N Points", &pointToRenderCount, 0, myModel->points.size());
	if (myModel != nullptr || triangleMeshTreeLoader.IsMeshLoaded()) ImGui::SliderInt("N Instances", &instanceCount, 0, treeInstancePositions.matrices.size());
	if (myModel != nullptr) {
		if (ImGui::Button("Shuffle")) {
			std::random_device rd;
			std::mt19937 g(rd());

			std::shuffle(myModel->points.begin(), myModel->points.end(), g);
			myModel->CopyPointsToVRAM();
		}
	}
	ImGui::Checkbox("FXAA", &fxaaEnabled);
	ImGui::Checkbox("TAA", &taaEnabled);
	ImGui::Checkbox("Logarithmic Colour Space", &taaLogarithmicColorSpace);
	ImGui::Checkbox("Mesh Render Instead", &meshRenderOn);
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

	ImGui::Begin("Live LOD Edits");

	if (myModel != nullptr && myModel->levelOfDetailType == PointTreeMesh::LODType::CONTINUOUS) {
		ImGui::DragFloat("Level 0 Points", &myModel->continousLOD_start, 128, 0, myModel->points.size());
		ImGui::DragFloat("Shallowness", &myModel->continousLOD_shallowness, 1, 0, 100);
		ImGui::DragFloat("Decay", &myModel->continousLOD_decay, 0.1f, 1, 5);
	}

	ImGui::End();

	ImGui::Begin("Shadow Map");
	ImGui::Image(sun.GetShadowImageImGuiTex(), ImVec2(800, 800));
	ImGui::End();

	sun.RenderImGuiMenu(true);
}

void MainDisplayApp::ImageCaptureSequence()
{
	if (!captureUnderway) return;

	std::vector<ImageCaptureRule> rules{
		{"Render001a", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001b", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001c", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001d", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001e", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001f", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001g", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render001h", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 0, 0);
			instanceCount = 4000;
		} },
		{"Render002", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, -90, 0);
			instanceCount = 1000;
		} },
		{"Render003", [&]() {
			cameraTransform.position = HMM_V3(0, 30, 0);
			cameraTransform.euler = HMM_V3(-45, 90, 0);
			instanceCount = 100;
		} },

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
				Graphics::SaveSwapChainImageToFile("imagesout\\" + rules[i].name + ".bmp");
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
		Graphics::SaveSwapChainImageToFile("./Render003.bmp");
		stageImagesSaved++;
		captureUnderway = false;
		renderImGui = true;

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
			if (csv.mmap("nvperfout\\"+ rule.name +"\\nvperf_metrics_summary.csv")) {
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
}
