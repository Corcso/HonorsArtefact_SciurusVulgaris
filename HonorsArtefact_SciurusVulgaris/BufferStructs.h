#pragma once
#include "PCH.h"

struct WCP_Matrices {
	HMM_Mat4 world;
	HMM_Mat4 camera;
	HMM_Mat4 projection;
};

struct VP_Matrices {
	HMM_Mat4 camera;
	HMM_Mat4 projection;
};

struct InstancingInfo {
	uint32_t numberOfInstances;
	uint32_t instanceStride;
	uint32_t numberOfModels;
	uint32_t myModelNumber;
};

struct MeshletInfo {
	uint32_t numberOfMeshlets;
};

struct LODDataBuffer {
	HMM_Vec4 cameraPosition;
	int  maxVertexLevels[16][4];
	int maxLevel;
	float continousStart, continousShallowness, continousDecay;
	int lodType;
};

struct PointRenderDebugInfo{
	bool coverageDisplayEnabled;
};

struct FXAAInfo {
	HMM_Vec2 inverseImageSize;
	bool enabled;
};

struct TAAInfo {
	HMM_Vec2 currentJitter;
	HMM_Vec2 inverseImageSize;
	uint32_t enabled;
	uint32_t logarithmicColorSpace;
};