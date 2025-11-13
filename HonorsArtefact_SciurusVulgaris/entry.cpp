#include "PCH.h"
#include "Input.h"
#include "Graphics.h"
#include "MeshRenderer.h"
#include "PointRenderPipeline.h"

int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	Graphics::Initialize(800, 800, L"test");
	MeshRenderer meshRenderingPipeline;
	PointRenderPipeline pointRenderingPipeline;
	meshRenderingPipeline.CreateAll();
	pointRenderingPipeline.CreateAll();

	std::vector<TriListMesh> myTreeModel = TriListMesh::LoadMultiMeshFile("./models/SpeedTrees/SpeedTree.obj");

	Image myTreeTexture;
	myTreeTexture.CreateAndLoadImageFromFile("./models/SpeedTrees/singleAColor.png", VK_IMAGE_USAGE_SAMPLED_BIT);
	myTreeTexture.CreateImageView();

	Image myBarkTexture;
	myBarkTexture.CreateAndLoadImageFromFile("./models/Low Poly Trees Free - Nicholas-3D/trunk_color.jpeg", VK_IMAGE_USAGE_SAMPLED_BIT);
	myBarkTexture.CreateImageView();

	std::vector<size_t> sizes = { sizeof(WCP_Matrices), 0 };

	myTreeModel[0].CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	myTreeModel[0].GetDescriptorSet()->UpdateImageSampler(1, &myTreeTexture, meshRenderingPipeline.GetSampler());

	myTreeModel[1].CreateDescriptorSet(meshRenderingPipeline.GetDescriptorSetLayout(), meshRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());
	myTreeModel[1].GetDescriptorSet()->UpdateImageSampler(1, &myBarkTexture, meshRenderingPipeline.GetSampler());

	for (int mesh = 0; mesh < myTreeModel.size(); mesh++) {
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(0, 0, -5), HMM_V3(0, 1, 0));
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(0, 0, 5), HMM_V3(0, 1, 0));
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(5, 0, 0), HMM_V3(0, 1, 0));
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(-5, 0, 0), HMM_V3(0, 1, 0));
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(0, -5, 0), HMM_V3(0, 0, 1));
		meshRenderingPipeline.ExtractPoints(&myTreeModel[mesh], HMM_V3(0, 5, 0), HMM_V3(0, 0, 1));
	}
	//Graphics::instance.meshRenderer.CollapsePoints();
	meshRenderingPipeline.GetPointMeshOutput()->CopyPointsToVRAM();
	sizes = { sizeof(WCP_Matrices) };
	meshRenderingPipeline.GetPointMeshOutput()->CreateDescriptorSet(pointRenderingPipeline.GetDescriptorSetLayout(), pointRenderingPipeline.GetDescriptorSetLayoutInfo(), sizes.data());

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

		pointRenderingPipeline.Render(meshRenderingPipeline.GetPointMeshOutput());
		Graphics::FinishImGuiRender();
		pointRenderingPipeline.EndRender();
		Graphics::EndRender();
	}
	Graphics::WaitUntilGPUIdle();
	myTreeModel.clear();
	myTreeTexture.Destroy();
	myBarkTexture.Destroy();
	meshRenderingPipeline.Shutdown();
	pointRenderingPipeline.Shutdown();
	Graphics::Shutdown();
	return 0;
}