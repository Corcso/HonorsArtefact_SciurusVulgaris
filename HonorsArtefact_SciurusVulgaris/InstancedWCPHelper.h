#pragma once
#include "PCH.h"
#include "BufferStructs.h"
class InstancedWCPHelper
{
public:
	void LoadFromFile(std::string path);
	void SetViewAndProjection(HMM_Mat4 view, HMM_Mat4 projection);
	void ApplyAlternateTransform(HMM_Mat4 transform);
	uint64_t ApplyRandomRotation(uint64_t seed = 0);

	std::vector<WCP_Matrices> matrices;
};

