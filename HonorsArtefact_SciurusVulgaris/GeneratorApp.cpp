#include "PCH.h"
#include "GeneratorApp.h"
#include <random>
#include "Input.h"
#include "Graphics.h"




#include "Clock.h"

void GeneratorApp::Initialize(){
	
	meshRenderingPipeline.CreateAll();
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
		meshRenderingPipeline.GetPointMeshOutput()->SaveToFile("./models/output.fbx");

		extractPointsAtEndOfThisFrame = false;
	}
	
}

void GeneratorApp::Shutdown() {
	Graphics::WaitUntilGPUIdle();
	meshRenderingPipeline.Shutdown();
}