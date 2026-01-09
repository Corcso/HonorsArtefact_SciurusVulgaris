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
private:
	static Clock instance;

	std::chrono::high_resolution_clock highResClock;

	std::chrono::time_point<std::chrono::high_resolution_clock> thisFrameStartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameStartTime;

	float deltaTime;

	int framesThisSecond;
	float timeSinceLastFPSReset;
	int lastSecondFPS;

};

