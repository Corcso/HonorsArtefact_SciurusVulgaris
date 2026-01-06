#pragma once
#include "PCH.h"

class Transform
{
public:
	HMM_Mat4 matrix;

	void DisplayController(uint32_t id = 0);

	HMM_Vec3 position;
	HMM_Vec3 euler;
	HMM_Vec3 scale;
};

