#pragma once
#include "PCH.h"
#include "BufferStructs.h"
class InstancedWCPHelper
{
public:
	void LoadFromFile(std::string path);
	void SetViewAndProjection(HMM_Mat4 view, HMM_Mat4 projection);
	void ApplyAlternateTransform(HMM_Mat4 transform);

	std::vector<WCP_Matrices> matrices;
};

