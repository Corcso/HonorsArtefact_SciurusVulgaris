#include "PCH.h"
#include "Clock.h"

Clock Clock::instance;

void Clock::FullReset()
{
	instance.deltaTime = 0;
	instance.lastSecondFPS = 0;
	instance.framesThisSecond = 0;
	instance.timeSinceLastFPSReset = 0;
	instance.lastFrameStartTime = instance.highResClock.now();
	instance.thisFrameStartTime = instance.highResClock.now();
}

void Clock::Frame()
{
	// Capture Time Points
	instance.lastFrameStartTime = instance.thisFrameStartTime;
	instance.thisFrameStartTime = instance.highResClock.now();

	// Calculate Delta Time
	instance.deltaTime = (instance.thisFrameStartTime.time_since_epoch().count() - instance.lastFrameStartTime.time_since_epoch().count()) / 1000000000.0f;

	// Calculate FPS
	instance.timeSinceLastFPSReset += instance.deltaTime;
	instance.framesThisSecond++;
	if (instance.timeSinceLastFPSReset > 1.0f) {
		instance.lastSecondFPS = instance.framesThisSecond;
		instance.timeSinceLastFPSReset = 0.0f;
		instance.framesThisSecond = 0;
	}
}

float Clock::DeltaTime()
{
	return instance.deltaTime;
}

int Clock::GetFPS()
{
	return instance.lastSecondFPS;
}
