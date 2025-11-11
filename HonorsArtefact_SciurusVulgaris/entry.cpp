#include "PCH.h"
#include "Input.h"
#include "Graphics.h"

void AddPCData(PointMesh* add);

int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	Graphics::Initialize(800, 800, L"test");

	PointMesh* myPoints = new PointMesh();

	//AddPCData(&myPoints);
	myPoints->LoadFromFile("./models/Flower Point Cloud Photogrammetry - Moshe Caine/flowerPoints.ply");
	size_t uboBufferSize = sizeof(WCP_Matrices);
	myPoints->CreateDescriptorSet(Graphics::GetDescriptorSetLayout(), Graphics::GetDescriptorSetLayoutInfo(), &uboBufferSize);
	myPoints->CopyPointsToVRAM();

	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(0, 0, -5));
	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(0, 0, 5));
	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(5, 0, 0));
	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(-5, 0, 0));
	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(0, -2, 0));
	Graphics::instance.meshRenderer.ExtractPoints(Graphics::instance.myMesh, HMM_V3(0, 2, 0));
	Graphics::instance.meshRenderer.GetPointMeshOutput()->CopyPointsToVRAM();
	std::vector<size_t> sizes = { sizeof(WCP_Matrices) };
	Graphics::instance.meshRenderer.GetPointMeshOutput()->CreateDescriptorSet(Graphics::GetDescriptorSetLayout(), Graphics::GetDescriptorSetLayoutInfo(), sizes.data());

	while (true) {
		Input::Update();
		if(Input::ProcessEvents()) break;
		if (Input::IsKeyDown('K')) {
			std::cout << "WOAH";
		}

		// Render logic
		Graphics::BeginRender();
		Graphics::Render(Graphics::instance.meshRenderer.GetPointMeshOutput());
		Graphics::EndRender();

		if (Input::IsKeyPressed('I')) {
			Graphics::instance.meshRenderer.TEMP_TestImageData();
		}
	}
	Graphics::WaitUntilGPUIdle();
	delete myPoints;
	Graphics::Shutdown();
	return 0;
}