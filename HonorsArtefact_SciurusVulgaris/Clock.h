#pragma once
#include <chrono>

/// <summary>
/// Clock Singleton for frame time keeping, deltaTime and other time related counters. 
/// </summary>
class Clock
{
public:
	/// <summary>
	/// Fully reset the clock back to application start
	/// </summary>
	static void FullReset();

	Clock() { FullReset(); }

	/// <summary>
	/// Mark a frame, run this at least once a frame at the start
	/// </summary>
	static void Frame();

	/// <summary>
	/// Get DeltaTime, the time between frames
	/// </summary>
	static float DeltaTime();

	/// <summary>
	/// Get an FPS value
	/// </summary>
	static int GetFPS();

	/// <summary>
	/// Get the current frame number
	/// </summary>
	static uint32_t GetCurrentFrameNumber();
private:
	static Clock instance; // Singleton instance

	std::chrono::high_resolution_clock highResClock;

	std::chrono::time_point<std::chrono::high_resolution_clock> thisFrameStartTime;
	std::chrono::time_point<std::chrono::high_resolution_clock> lastFrameStartTime;

	float deltaTime;

	int framesThisSecond;
	float timeSinceLastFPSReset;
	int lastSecondFPS;

	uint32_t currentFrameNumber;

};

