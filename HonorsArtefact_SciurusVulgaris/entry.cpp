#include "PCH.h"
#include "Input.h"
#include "Graphics.h"
int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	Graphics::Initialize(400, 400, L"test");

	while (true) {
		Input::Update();
		Input::ProcessEvents();
		if (Input::IsKeyDown('K')) {
			std::cout << "WOAH";
		}
	}

	return 0;
}