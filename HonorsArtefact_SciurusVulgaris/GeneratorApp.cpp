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
	descriptorSizes = { sizeof(WCP_Matrices), 0 };
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
		size_t sizes = sizeof(WCP_Matrices);
		LODViewDescriptors[i].Create(debugPointRenderer.GetDescriptorSetLayout(), debugPointRenderer.GetDescriptorSetLayoutInfo(), &sizes);

		LODViewDescriptors[i].UpdateUniformBufferData(0, &newData);
	}
}

void GeneratorApp::Frame() {
	// Render logic
	Graphics::BeginRender();
	meshRenderingPipeline.BeginRender(HMM_V4(0, 0, 0, 1));
	for (auto& mesh : loadedModel) {
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
	ImGui::InputText("Model Path", modelPath, 256);
	ImGui::Text("Textures");
	for (int i = 0; i < 8; i++) {
		ImGui::PushID(i);
		ImGui::PushID(1);
		ImGui::Checkbox("", &imageActive[i]);
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::PushID(2);
		ImGui::InputText("", texturePaths[i], 256);
		ImGui::PopID();
		ImGui::PopID();
	}
	if (ImGui::Button("Load")) {
		loadedModel.clear();
		loadedImages.clear();
		loadedImages.resize(8);

		for (int i = 0; i < 8; i++) {
			if (imageActive[i]) {
				loadedImages[i].CreateAndLoadImageFromFile(texturePaths[i], VK_IMAGE_USAGE_SAMPLED_BIT);
				loadedImages[i].CreateImageView();
			}
		}

		loadedModel = TriListMesh::LoadMultiMeshFile(modelPath);
		for (int i = 0; i < loadedModel.size(); i++) {
			loadedModel[i].CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), descriptorSizes.data());
			loadedModel[i].GetDescriptorSet()->UpdateImageSampler(1, &loadedImages[i], meshRenderingPipeline.GetSampler());
		}
	}
	if (ImGui::Button("Execute Point Generation")) {
		extractPointsAtEndOfThisFrame = true;
	}
	ImGui::Checkbox("Debug : Constant Extraction", &extractPointsConstantly);
	if (ImGui::Button("Exit to Point Renderer")) {
		quitMainLoop = true;
	}
	loadedModelTransform.DisplayController();
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
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		dataForUBO = {
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, 0, 5), HMM_V3(0, 0, 10), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		dataForUBO = {
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(-5, 0, 0), HMM_V3(-10, 0, 0), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		dataForUBO = {
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(5, 0, 0), HMM_V3(10, 0, 0), HMM_V3(0, -1, 0)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		dataForUBO = {
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, 5, 0), HMM_V3(0, 10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		dataForUBO = {
				HMM_Translate(HMM_V3(0, -9.5, 0)) * HMM_Rotate_LH(3.141 / 2.0, HMM_V3(1, 0, 0)) * HMM_Scale(HMM_V3(0.3, 0.3, 0.3)), HMM_LookAt_LH(HMM_V3(0, -5, 0), HMM_V3(0, -10, 0), HMM_V3(0, 0, -1)), HMM_Orthographic_RH_ZO(-10, 10, -10, 10, 0.001, 10)
		};
		meshRenderingPipeline.ExtractPointsNew(&loadedModel, dataForUBO);
		//meshRenderingPipeline.GetPointMeshOutput()->SaveToFile("./models/output.fbx");
		
		meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
		//size_t sizes = sizeof(WCP_Matrices);
		//meshRenderingPipeline.GetPointMeshOutput()->GetDescriptorSet()->Create(debugPointRenderer.GetDescriptorSetLayout(), debugPointRenderer.GetDescriptorSetLayoutInfo(), &sizes);

		extractPointsAtEndOfThisFrame = false;
	}
	
}

void GeneratorApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	meshRenderingPipeline.Shutdown();
	debugPointRenderer.Shutdown(); 
	for (int i = 0; i < 16; i++) {
		LODViewDescriptors[i].CleanupDescriptor();
	}
}

void GeneratorApp::RenderLODPagePrerequisites()
{
	if (!meshRenderingPipeline.GetPointMeshOutput()->isDataOnGPU) return;
	debugPointRenderer.BeginRender(HMM_V4(0, 0, 0, 1));
	for (int i = 0; i < meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size(); i++) {
		//meshRenderingPipeline.GetPointMeshOutput()->GetDescriptorSet()->UpdateUniformBufferData(0, &newData);
		VkRect2D view{
			{i % 4 * 256, i / 4 * 256}, {256, 256}
		};
		debugPointRenderer.Render(meshRenderingPipeline.GetPointMeshOutput(), view, i, &LODViewDescriptors[i]);
	}
	debugPointRenderer.EndRender();
}

void GeneratorApp::RenderLODPageMenu()
{
	ImGui::Begin("Level Of Detail View");
	ImGui::Image(debugPointRenderer.GetImGuiOutputTexture(), ImVec2{ 700, 700 });
	ImGui::End();
	ImGui::Begin("Level Of Detail Studio");
	if (ImGui::Button("Reshuffle Points"))
	{
		std::random_device rd;
		std::mt19937 g(rd());

		std::shuffle(meshRenderingPipeline.GetPointMeshOutput()->points.begin(), meshRenderingPipeline.GetPointMeshOutput()->points.end(), g);
		meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
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
	ImGui::SliderAngle("Rotation", &LODViewRotation);
	for (int i = 0; i < meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount.size(); i++) {
		ImGui::DragFloat(("Distance " + std::to_string(i)).c_str(), &LODViewDistances[i]);
		ImGui::SameLine();
		ImGui::InputScalar(("Point Count LOD " + std::to_string(i)).c_str(), ImGuiDataType_U32, &meshRenderingPipeline.GetPointMeshOutput()->randomLevelsLODPointCount[i]);
	}
	if (ImGui::Button("Save Tree File")) {
		meshRenderingPipeline.GetPointMeshOutput()->SaveToTreeFile("./models/output.tree");
	}
	
	ImGui::End();


	for (int i = 0; i < 16; i++) {
		WCP_Matrices newData{
				HMM_Translate(HMM_V3(0, 0, LODViewDistances[i])) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(LODViewRotation, HMM_V3(0, 1, 0)),
				HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)),
				HMM_Perspective_RH_ZO(70 * HMM_DegToRad, 1, 0.001, 100)
		};

		LODViewDescriptors[i].UpdateUniformBufferData(0, &newData);
	}
}
