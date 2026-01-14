#include "PCH.h"

#include "Input.h"
#include "Clock.h"
#include "Graphics.h"

#include "GeneratorApp.h"
#include "MainDisplayApp.h"

int main() {
	Input::Initialize();
	Graphics::Initialize(800, 800, L"Artefact");
	Clock::FullReset();

	// GENERATOR
	GeneratorApp generator;
	generator.Initialize();
	while (true) {
		Input::Update();
		if (Input::ProcessEvents()) break;

		generator.Frame();
		if (generator.quitMainLoop) break;
	}
	generator.Shutdown();

	// DISPLAY
	MainDisplayApp displayApp;
	displayApp.Initialize();
	while (true) {
		Clock::Frame();
		Input::Update();
		if (Input::ProcessEvents()) break;

		displayApp.Frame();
		if (displayApp.quitMainLoop) break;
	}
	displayApp.Shutdown();

	Graphics::WaitUntilGPUIdle();
	Graphics::Shutdown();
	return 0;
}