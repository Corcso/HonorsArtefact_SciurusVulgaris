#include "PCH.h"
#include "Input.h"
int main() {
	std::cout << "I'm Alive";

	Input::Initialize();
	while (true) {
		Input::Update();
		Input::ProcessEvents();
		if (Input::IsKeyDown('K')) {
			std::cout << "WOAH";
		}
	}

	return 0;
}