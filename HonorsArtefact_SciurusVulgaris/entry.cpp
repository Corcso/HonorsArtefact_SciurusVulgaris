#include "PCH.h"
#include "Input.h"
#include "Graphics.h"
#include "MeshRenderer.h"
#include "PointRenderPipeline.h"
#include "InstancedTreeRenderPass.h"
#include "ImGuiBlankRenderPass.h"
#include "Transform.h"
void GeneratorApp();
void DisplayApp();
void InstancedDisplayApp();

int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	Graphics::Initialize(800, 800, L"test");

	GeneratorApp();
	InstancedDisplayApp();

	Graphics::WaitUntilGPUIdle();
	Graphics::Shutdown();
	return 0;

	MeshRenderer meshRenderingPipeline;
	PointRenderPipeline pointRenderingPipeline;
	meshRenderingPipeline.CreateAll();
	pointRenderingPipeline.CreateAll();

	// DELME
	/*PointMesh* vase = new PointMesh();
	vase->LoadFromFile("./models/Flower Point Cloud Photogrammetry - Moshe Caine/flowerPoints.ply");
	delete vase;*/
	// END

	//std::vector<TriListMesh> myTreeModel = TriListMesh::LoadMultiMeshFile("./models/SpeedTrees/SpeedTree.obj");

	//Image myTreeTexture;
	//myTreeTexture.CreateAndLoadImageFromFile("./models/SpeedTrees/singleAColor.png", VK_IMAGE_USAGE_SAMPLED_BIT);
	//myTreeTexture.CreateImageView();

	//Image myBarkTexture;
	//myBarkTexture.CreateAndLoadImageFromFile("./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", VK_IMAGE_USAGE_SAMPLED_BIT);
	//myBarkTexture.CreateImageView();

	std::vector<size_t> sizes = { sizeof(WCP_Matrices), 0 };

	//myTreeModel[0].CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	//myTreeModel[0].GetDescriptorSet()->UpdateImageSampler(1, &myTreeTexture, meshRenderingPipeline.GetSampler());

	//myTreeModel[1].CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	//myTreeModel[1].GetDescriptorSet()->UpdateImageSampler(1, &myBarkTexture, meshRenderingPipeline.GetSampler());

	//TriListMesh* referenceArray[2]{ &myTreeModel[0] , &myTreeModel[1] };
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(0, 0, -5), HMM_V3(0, 1, 0));
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(0, 0, 5), HMM_V3(0, 1, 0));
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(5, 0, 0), HMM_V3(0, 1, 0));
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(-5, 0, 0), HMM_V3(0, 1, 0));
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(0, -5, 0), HMM_V3(0, 0, 1));
	//meshRenderingPipeline.ExtractPoints(referenceArray, 2, HMM_V3(0, 5, 0), HMM_V3(0, 0, 1));
	//
	////Graphics::instance.meshRenderer.CollapsePoints();
	//meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
	sizes = { sizeof(WCP_Matrices) };
	//meshRenderingPipeline.GetPointMeshOutput()->CreateDescriptorSet(pointRenderingPipeline.GetDescriptorSetLayout(), pointRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	//meshRenderingPipeline.GetPointMeshOutput()->SaveToFile("./models/SpeedTrees/output.fbx");
	
	PointMesh* testLoaded = new PointMesh();
	testLoaded->LoadFromFile("./models/SpeedTrees/output.fbx");
	testLoaded->CopyPointsToVRAM();
	testLoaded->CreateDescriptorSet(pointRenderingPipeline.GetDescriptorSetLayout(), pointRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	
	while (true) {
		Input::Update();
		if(Input::ProcessEvents()) break;
		if (Input::IsKeyDown('K')) {
			std::cout << "WOAH";
		}

		// Render logic
		Graphics::BeginRender();
		//Graphics::Render(Graphics::instance.meshRenderer.GetPointMeshOutput());
		pointRenderingPipeline.BeginRender(HMM_V4(0, 0, 0, 1));

		pointRenderingPipeline.Render(testLoaded);
		Graphics::FinishImGuiRender();
		pointRenderingPipeline.EndRender();
		Graphics::EndRender();
	}
	Graphics::WaitUntilGPUIdle();
	//myTreeModel.clear();
	//myTreeTexture.Destroy();
	//myBarkTexture.Destroy();
	delete testLoaded;
	meshRenderingPipeline.Shutdown();
	pointRenderingPipeline.Shutdown();
	Graphics::Shutdown();
	return 0;
}

void DisplayApp() {
	PointRenderPipeline pointRenderingPass;
	pointRenderingPass.CreateAll();
	std::vector<size_t> descriptorSizes = { sizeof(WCP_Matrices) };

	PointMesh* myModel = nullptr;
	char modelPath[256] = "./models/output.fbx";
	float angle = 0;

	CameraTransform cameraTransform;
	cameraTransform.position = HMM_V3(0, 0, -5);

	while (true) {
		Input::Update();
		if (Input::ProcessEvents()) break;

		cameraTransform.CaptureControls();

		// Render logic
		Graphics::BeginRender();
		pointRenderingPass.BeginRender(HMM_V4(0, 0, 0, 1));

		if (myModel != nullptr) {
			WCP_Matrices dataForUBO;

			dataForUBO = {
				//HMM_M4D(1) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Perspective_RH_ZO(70, 1, 0.001, 10)
				HMM_M4D(1) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)), cameraTransform.viewMatrix, HMM_Perspective_RH_ZO(70, 1, 0.001, 100)
			};
			
			myModel->GetDescriptorSet()->UpdateUniformBufferData(0, &dataForUBO);
			pointRenderingPass.Render(myModel);
		}

		ImGui::Begin("Point Model Selection");
		ImGui::InputText("Model Path", modelPath, 256);
		if (ImGui::Button("Load")) {
			if (myModel != nullptr) delete myModel;

			myModel = new PointMesh();

			myModel->LoadFromFile(modelPath);
			myModel->CopyPointsToVRAM();
			myModel->CreateDescriptorSet(pointRenderingPass.GetDescriptorSetLayout(), pointRenderingPass.GetDescriptorSetLayoutInfo(), descriptorSizes.data());
		}
		ImGui::SliderAngle("Angle", &angle);
		ImGui::End();

		Graphics::FinishImGuiRender();
		pointRenderingPass.EndRender();
		Graphics::EndRender();
	}
	Graphics::WaitUntilGPUIdle();
	if (myModel != nullptr) delete myModel;
	pointRenderingPass.Shutdown();
}

void GeneratorApp() {
	MeshRenderer meshRenderingPipeline;
	ImGuiBlankRenderPass imguiRenderPass;
	meshRenderingPipeline.CreateAll();
	imguiRenderPass.CreateAll(); // Not needed but just incase stuff is added later
	std::vector<size_t> descriptorSizes = { sizeof(WCP_Matrices), 0 };
	ImTextureID liveColorOut = reinterpret_cast<ImTextureID>(ImGui_ImplVulkan_AddTexture(meshRenderingPipeline.GetSampler(), meshRenderingPipeline.GetColorImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

	std::vector<TriListMesh> loadedModel;
	std::vector<Image> loadedImages;
	Transform loadedModelTransform;
	loadedModelTransform.position = HMM_V3(0, -9.5, 0);
	loadedModelTransform.euler = HMM_V3(0, 90, 0);
	loadedModelTransform.scale = HMM_V3(0.3, 0.3, 0.3);


	std::string loadStatus;
	char modelPath[256] = "./models/SpeedTrees/SpeedTree.obj";
	bool imageActive[8]{ true, true, false, false, false, false, false, false };
	char texturePaths[8][256]{"./models/SpeedTrees/singleAColor.png", "./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", "", "",  "",  "",  "",  "", };
	bool isTopView = false;

	bool extractPointsAtEndOfThisFrame = false; // Will flip true when points should be extracted, and save them to file. 
	bool extractPointsConstantly = false; // Extracts and saves points every frame, just used for easy render doc capture
	bool exitAtThisFrameEnd = false;
	while (!exitAtThisFrameEnd) {
		Input::Update();
		if (Input::ProcessEvents()) break;

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
			exitAtThisFrameEnd = true;
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

		if(extractPointsAtEndOfThisFrame || extractPointsConstantly){
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
	Graphics::WaitUntilGPUIdle();
	meshRenderingPipeline.Shutdown();
}

void InstancedDisplayApp() {
	InstancedTreeRenderPass pointRenderingPass;
	pointRenderingPass.CreateAll();
	std::vector<size_t> descriptorSizes = { sizeof(WCP_Matrices) * 100 };

	PointMesh* myModel = nullptr;
	char modelPath[256] = "./models/output.fbx";
	float angle = 0;

	CameraTransform cameraTransform;
	cameraTransform.position = HMM_V3(0, 0, -5);

	while (true) {
		Input::Update();
		if (Input::ProcessEvents()) break;

		cameraTransform.CaptureControls();

		// Render logic
		Graphics::BeginRender();
		pointRenderingPass.BeginRender(HMM_V4(0, 0, 0, 1));

		if (myModel != nullptr) {
			std::vector<WCP_Matrices> dataForUBO(400);

			for (int x = 0; x < 20; x++) {
				for (int y = 0; y < 20; y++) {
					dataForUBO[x * 10 + y] = {
						//HMM_M4D(1) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)), HMM_LookAt_LH(HMM_V3(0, 0, -5), HMM_V3(0, 0, -10), HMM_V3(0, -1, 0)), HMM_Perspective_RH_ZO(70, 1, 0.001, 10)
						HMM_Translate(HMM_V3(x * 5.1f, 0, y * 5.1f)) * HMM_Scale(HMM_V3(0.2, 0.2, 0.2)) * HMM_Rotate_LH(angle, HMM_V3(0, 1, 0)),
						cameraTransform.viewMatrix, 
						HMM_Perspective_RH_ZO(70, 1, 0.001, 100)
					};
				}
			}	

			myModel->GetDescriptorSet()->UpdateUniformBufferData(0, dataForUBO.data());
			pointRenderingPass.Render(myModel);
		}

		ImGui::Begin("Point Model Selection");
		ImGui::InputText("Model Path", modelPath, 256);
		if (ImGui::Button("Load")) {
			if (myModel != nullptr) delete myModel;

			myModel = new PointMesh();

			myModel->LoadFromFile(modelPath);
			myModel->CopyPointsToVRAM();
			myModel->CreateDescriptorSet(pointRenderingPass.GetDescriptorSetLayout(), pointRenderingPass.GetDescriptorSetLayoutInfo(), descriptorSizes.data());
		}
		ImGui::SliderAngle("Angle", &angle);
		ImGui::End();

		Graphics::FinishImGuiRender();
		pointRenderingPass.EndRender();
		Graphics::EndRender();
	}
	Graphics::WaitUntilGPUIdle();
	if (myModel != nullptr) delete myModel;
	pointRenderingPass.Shutdown();
}