#pragma once
#include "PCH.h"

// Various structs for shader buffers

/// <summary>
/// World Camera Projection matrices
/// </summary>
struct WCP_Matrices {
	HMM_Mat4 world;
	HMM_Mat4 camera;
	HMM_Mat4 projection;
};

/// <summary>
/// View(Camera) Projection matrices
/// </summary>
struct VP_Matrices {
	HMM_Mat4 camera;
	HMM_Mat4 projection;
};

/// <summary>
/// Instancing information
/// </summary>
struct InstancingInfo {
	uint32_t numberOfInstances;
	uint32_t instanceStride;
	uint32_t numberOfModels;
	uint32_t myModelNumber;
};

/// <summary>
/// Meshlet information
/// </summary>
struct MeshletInfo {
	uint32_t numberOfMeshlets;
};

/// <summary>
/// LOD Information
/// </summary>
struct LODDataBuffer {
	HMM_Vec4 cameraPosition;
	int  maxVertexLevels[16][4];
	int maxLevel;
	float continousStart, continousShallowness, continousDecay;
	int lodType;
};

/// <summary>
/// Point Rendering Debug Info (for LOD Studio in GeneratorApp)
/// </summary>
struct PointRenderDebugInfo{
	bool coverageDisplayEnabled;
};

/// <summary>
/// FXAA Info
/// </summary>
struct FXAAInfo {
	HMM_Vec2 inverseImageSize;
	bool enabled;
};

/// <summary>
/// TAA Info
/// </summary>
struct TAAInfo {
	HMM_Vec2 currentJitter;
	HMM_Vec2 inverseImageSize;
	uint32_t enabled;
	uint32_t logarithmicColorSpace;
};