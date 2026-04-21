#pragma once
#include "PointMesh.h"

/// <summary>
/// A wrapper around PointMesh which stores LOD related data.
/// </summary>
class PointTreeMesh :
    public PointMesh
{
public:
    /// <summary>
    /// Enum of possible LOD types
    /// </summary>
    enum class LODType {
        RANDOM_LEVELS, CONTINUOUS
    };

    /// <summary>
    /// Load from a .tree file. A JSON file which contains LOD information
    /// <para>Also loads the model under filename_MODELDATA.???</para>
    /// </summary>
    void LoadFromTreeFile(std::string path);

    /// <summary>
    /// Save to a .tree file. A JSON file which contains LOD information
    /// <para>Also saves the model under filename_MODELDATA.fbx</para>
    /// </summary>
    void SaveToTreeFile(std::string path);

    LODType levelOfDetailType;
    std::vector<unsigned int> randomLevelsLODPointCount;
    float continousLOD_shallowness;
    float continousLOD_decay;
    float continousLOD_start;
};

