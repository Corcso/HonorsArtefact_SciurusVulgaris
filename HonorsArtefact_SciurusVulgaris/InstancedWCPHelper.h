#pragma once
#include "PCH.h"
#include "BufferStructs.h"
/// <summary>
/// A helper which loads a large list of positions from a OBJ file (each vertex as a position)
/// <para>Used for tree placement</para>
/// </summary>
class InstancedWCPHelper
{
public:
	/// <summary>
	/// Load position list from file
	/// </summary>
	void LoadFromFile(std::string path);
	/// <summary>
	/// Set view and projection matrices for WCP Structs
	/// </summary>
	void SetViewAndProjection(HMM_Mat4 view, HMM_Mat4 projection);
	/// <summary>
	/// Apply a transform ontop of the world matrix.
	/// </summary>
	void ApplyAlternateTransform(HMM_Mat4 transform);
	/// <summary>
	/// Apply a random rotation to each position.
	/// </summary>
	/// <returns>The seed used</returns>
	uint64_t ApplyRandomRotation(uint64_t seed = 0);

	std::vector<WCP_Matrices> matrices;
};

/// <summary>
/// Same as WCP Helper without View or Projection Matrices
/// <para>See InstancedWCPHelper for function useage</para>
/// </summary>
class InstancedWorldMatrixHelper
{
public:
	void LoadFromFile(std::string path);
	void ApplyAlternateTransform(HMM_Mat4 transform);
	uint64_t ApplyRandomRotation(uint64_t seed = 0);

	std::vector<HMM_Mat4> matrices;
};

