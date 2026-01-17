#pragma once
#include "PCH.h"

struct WCP_Matrices {
	HMM_Mat4 world;
	HMM_Mat4 camera;
	HMM_Mat4 projection;
};

struct LODDataBuffer {
	HMM_Vec4 cameraPosition;
	int  maxVertexLevels[16][4];
	int maxLevel;
};