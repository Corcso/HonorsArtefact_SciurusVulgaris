#pragma once
#include <chrono>
class Clock
{
public:
	static void FullReset();

	Clock() { FullReset(); }

	static void Frame();

	static float DeltaTime();

	static int GetFPS();

	static uint32_t GetCurrentFrameNumber();
private:
	static Clock instance;

	std::chrono::high_resolution_clock highResClock;

	std::chrono::time_point<std::chrono::high_resolution_clock> thisFrameStartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameStartTime;

	float deltaTime;

	int framesThisSecond;
	float timeSinceLastFPSReset;
	int lastSecondFPS;

	uint32_t currentFrameNumber;

};

