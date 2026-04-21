#include "PCH.h"
#include "GeneratorApp.h"
#include <random>
#include "Input.h"
#include "Graphics.h"




#include "Clock.h"

void GeneratorApp::Initialize(){
	
	meshRenderingPipeline.CreateAll();
	debugPointRenderer.CreateAll();
	imguiRenderPass.CreateAll(); // Not needed but just incase stuff is added later
	descriptorSizes = { sizeof(WCP_Matrices), 0, sizeof(TAAInfo)};
	liveColorOut = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(meshRenderingPipeline.GetSampler(), meshRenderingPipeline.GetColorImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	
	loadedModelTransform.position = HMM_V3(0, -9.5, 0);
	loadedModelTransform.euler = HMM_V3(0, 90, 0);
	loadedModelTransform.scale = HMM_V3(0.3, 0.3, 0.3);

	//modelPath[256] = "./models/SpeedTrees/SpeedTree.obj";
	//imageActive[8]{ true, true, false, false, false, false, false, false };
	//texturePaths[8][256]{ "./models/SpeedTrees/singleAColor.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", };
	isTopView = false;

	extractPointsAtEndOfThisFrame = false; // Will flip true when points should be extracted, and save them to file. 
	extractPointsConstantly = false; // Extracts and saves points every frame, just used for easy render doc capture
	quitMainLoop = false;

	LODViewDescriptors.resize(16);
	LODViewDistances.resize(16);
	for (int i = 0; i < 16; i++) {
		LODViewDistances[i] = 0;
		WCP_Matrices newData{
				HMM_Scale(HMM_V3(0.2, 0.2, 0.2)),
				HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)),
				HMM_Perspective_RH_ZO((70 + (i * 5)) * HMM_DegToRad, 1, 0.001, 100)
		};
		size_t sizes[] = { sizeof(WCP_Matrices), sizeof(PointRenderDebugInfo) };
		LODViewDescriptors[i].Create(debugPointRenderer.GetDescriptorSetLayout(), debugPointRenderer.GetDescriptorSetLayoutInfo(), sizes);

		LODViewDescriptors[i].UpdateUniformBufferData(0, &newData);
	}
}

void GeneratorApp::Frame() {
	// Render logic
	Graphics::BeginRender();
	meshRenderingPipeline.BeginRender(HMM_V4(0, 0, 0, 1));
	for (auto& mesh : *treeMeshLoader.GetMeshVector()) {
		WCP_Matrices dataForUBO;
		if (isTopView) {
			dataForUBO = {
				//HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, 5, 0), HMM_V3(0, 10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, 5, 0), HMM_V3(0, 10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
			};
		}
		else {
			dataForUBO = {
				//HMM_Translate(HMM_V3(0, -9.5, 0))* HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
			};
		}
		mesh.GetDescriptorSet()->UpdateUniformBufferData(0, &dataForUBO);
		meshRenderingPipeline.Render(&mesh);
	}
	meshRenderingPipeline.EndRender();

	RenderLODPagePrerequisites();

	// ImGui
	imguiRenderPass.BeginRender(HMM_V4(0, 0, 0, 1));
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

	ImGui::Begin("Model Selection");
	treeMeshLoader.Display([&](TriListMesh* mesh, Image* texture) {
		mesh->CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), descriptorSizes.data());
		mesh->GetDescriptorSet()->UpdateImageSampler(1, texture, meshRenderingPipeline.GetSampler());
	});
	if (treeMeshLoader.IsMeshLoaded()) {
		loadedModelTransform.DisplayController();
		if (ImGui::Button("Execute Point Generation")) {
			extractPointsAtEndOfThisFrame = true;
		}
		ImGui::Checkbox("Debug : Constant Extraction", &extractPointsConstantly);
	}
	if (ImGui::Button("Exit to Point Renderer")) {
		quitMainLoop = true;
	}
	ImGui::End();

	ImGui::Begin("Live Screen");
	ImGui::Checkbox("Top View", &isTopView);
	ImGui::Image(liveColorOut, ImVec2(512, 512));
	ImGui::End();

	if (meshRenderingPipeline.GetPointMeshOutput()->isDataOnGPU)	RenderLODPageMenu();

	Graphics::FinishImGuiRender();
	imguiRenderPass.EndRender();
	Graphics::EndRender();

	if (extractPointsAtEndOfThisFrame || extractPointsConstantly) {
		WCP_Matrices dataForUBO;
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, 0, 5), HMM_V3(0, 0, 10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(-5, 0, 0), HMM_V3(-10, 0, 0), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(5, 0, 0), HMM_V3(10, 0, 0), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, 5, 0), HMM_V3(0, 10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		dataForUBO = {
				loadedModelTransform.matrix, HMM_LookAt_LH(HMM_V3(0, -5, 0), HMM_V3(0, -10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(treeMeshLoader.GetMeshVector(), dataForUBO);
		//meshRenderingPipeline.GetPointMeshOutput()->SaveToFile("./models/output.fbx");
		
		meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
		//size_t sizes = sizeof(WCP_Matrices);
		//meshRenderingPipeline.GetPointMeshOutput()->GetDescriptorSet()->Create(debugPointRenderer.GetDescriptorSetLayout(), debugPointRenderer.GetDescriptorSetLayoutInfo(), &sizes);

		extractPointsAtEndOfThisFrame = false;
	}

	if (shuffleAtEndOfFrame) {
		std::random_device rd;
		std::mt19937 g(rd());
		vkDeviceWaitIdle(Graphics::GetVkDevice());
		std::shuffle(meshRenderingPipeline.GetPointMeshOutput()->points.begin(), meshRenderingPipeline.GetPointMeshOutput()->points.end(), g);
		meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
		shuffleAtEndOfFrame = false;
	}
	
}

void GeneratorApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	meshRenderingPipeline.Shutdown();
	debugPointRenderer.Shutdown(); 
	for (int i = 0; i < 16; i++) {
		LODViewDescriptors[i].CleanupDescriptor();
	}
	//for (auto& texture : loadedImages) texture.Destroy();
	treeMeshLoader.Cleanup();
}

void GeneratorApp::RenderLODPagePrerequisites()
{
	if (!meshRenderingPipeline.GetPointMeshOutput()->isDataOnGPU) return;

	PointRenderDebugInfo debugInfo{ isDebugCoverageViewOn };
	for (int i = 0; i < 16; i++) {
		LODViewDescriptors[i].UpdateUniformBufferData(1, &debugInfo);
	}
	
	debugPointRenderer.BeginRender(HMM_V4(0, 0, 0, 1));
	if (exclusivleyViewing == -1) {
		for (int i = 0; i < meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size(); i++) {
			//meshRenderingPipeline.GetPointMeshOutput()->GetDescriptorSet()->UpdateUniformBufferData(0, &newData);
			VkRect2D view{
				{i % 4 * 256, i / 4 * 256}, {256, 256}
			};
			debugPointRenderer.Render(meshRenderingPipeline.GetPointMeshOutput(), view, i, &LODViewDescriptors[i], LODViewDistances[i]);
		}
	}
	else {
		VkRect2D view{
				{0, 0}, {1024, 1024}
		};
		debugPointRenderer.Render(meshRenderingPipeline.GetPointMeshOutput(), view, exclusivleyViewing, &LODViewDescriptors[exclusivleyViewing], LODViewDistances[exclusivleyViewing]);
	}
	debugPointRenderer.EndRender();
}

void GeneratorApp::RenderLODPageMenu()
{
	ImGui::Begin("Level Of Detail View");
	ImGui::Image(debugPointRenderer.GetImGuiOutputTexture(), ImVec2{ 700, 700 });
	ImGui::End();
	ImGui::Begin("Level Of Detail Studio");
	ImGui::Checkbox("Coverage View", &isDebugCoverageViewOn);
	ImGui::DragFloat("Camera Height", &LODCameraHeight, 0.01f, -100.0f, 100.0f);
	ImGui::DragFloat("Scale", &LODViewScale, 0.0001f, 0.001f, 100.0f);
	ImGui::SliderAngle("Rotation", &LODViewRotation);
	ImGui::DragInt("Exclusive View", &exclusivleyViewing, 1, -1, 15);
	if (ImGui::Button("Use Random Levels"))
	{
		meshRenderingPipeline.GetPointMeshOutput()->levelOfDetailType = PointTreeMesh::LODType::RANDOM_LEVELS;
	}
	if (ImGui::Button("Use Continuous"))
	{
		meshRenderingPipeline.GetPointMeshOutput()->levelOfDetailType = PointTreeMesh::LODType::CONTINUOUS;
	}
	if (ImGui::Button("Reshuffle Points"))
	{
		shuffleAtEndOfFrame = true;
	}
	if (ImGui::Button("-") && meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size() > 0) {
		meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.pop_back();
	}
	ImGui::SameLine();
	if (ImGui::Button("+") && meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size() < 16) {
		if (meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size() < 1) {
			meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.push_back(
				meshRenderingPipeline.GetPointMeshOutput()->points.size()
			);
		}
		else {
			meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.push_back(
				meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount[meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size() - 1] / 2
			);
		}
	}
	if (meshRenderingPipeline.GetPointMeshOutput()->levelOfDetailType == PointTreeMesh::LODType::CONTINUOUS) {
		ImGui::DragFloat("Level 0 Points", &meshRenderingPipeline.GetPointMeshOutput()->continousLOD_start, 128, 0, meshRenderingPipeline.GetPointMeshOutput()->points.size());
		ImGui::DragFloat("Shallowness", &meshRenderingPipeline.GetPointMeshOutput()->continousLOD_shallowness, 1, 0, 100);
		ImGui::DragFloat("Decay", &meshRenderingPipeline.GetPointMeshOutput()->continousLOD_decay, 0.1f, 1, 5);
	}
	for (int i = 0; i < meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size(); i++) {
		ImGui::DragFloat(("Distance " + std::to_string(i)).c_str(), &LODViewDistances[i]);
		if (meshRenderingPipeline.GetPointMeshOutput()->levelOfDetailType == PointTreeMesh::LODType::RANDOM_LEVELS) {
			ImGui::SameLine();
			ImGui::InputScalar(("Point Count LOD " + std::to_string(i)).c_str(), ImGuiDataType_U32, &meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount[i]);
		}
	}
	if (ImGui::Button("Save Tree File")) {
		meshRenderingPipeline.GetPointMeshOutput()->SaveToTreeFile("./models/output.tree");
	}
	
	ImGui::End();


	for (int i = 0; i < 16; i++) {
		WCP_Matrices newData{
				loadedModelTransform.matrix * HMM_Translate(HMM_V3(LODViewDistances[i], 0, 0)) * HMM_Rotate_LH(LODViewRotation, HMM_V3(0, 1, 0)) * HMM_Scale(HMM_V3(LODViewScale, LODViewScale, LODViewScale)),
				HMM_LookAt_LH(HMM_V3(0, LODCameraHeight, 0), HMM_V3(0, LODCameraHeight, -1), HMM_V3(0, -1, 0)),
				HMM_Perspective_RH_ZO(50 * HMM_DegToRad, 1, 0.001, 100)
		};

		LODViewDescriptors[i].UpdateUniformBufferData(0, &newData);
	}
}
