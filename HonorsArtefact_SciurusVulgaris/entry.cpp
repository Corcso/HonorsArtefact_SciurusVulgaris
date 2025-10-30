#include "PCH.h"
#include "Input.h"
#include "Graphics.h"

void AddPCData(PointMesh* add);

int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	Graphics::Initialize(800, 800, L"test");

	PointMesh myPoints;

	//AddPCData(&myPoints);
	myPoints.LoadFromFile("./models/Flower Point Cloud Photogrammetry - Moshe Caine/flowerPoints.ply");

	myPoints.CopyPointsToVRAM();

	while (true) {
		Input::Update();
		Input::ProcessEvents();
		if (Input::IsKeyDown('K')) {
			std::cout << "WOAH";
		}

		// Render logic
		Graphics::BeginRender();
		Graphics::Render(&myPoints);
		Graphics::EndRender();
	}

	return 0;
}