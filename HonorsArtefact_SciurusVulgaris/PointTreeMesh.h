#pragma once
#include "PointMesh.h"
class PointTreeMesh :
    public PointMesh
{
public:
    enum class LODType {
        RANDOM_LEVELS, CONTINUOUS
    };

    void LoadFromTreeFile(std::string path);
    void SaveToTreeFile(std::string path);

    LODType levelOfDetailType;
    std::vector<unsigned int> randomLevelsLODPointCount;
    float continousLOD_shallowness;
    float continousLOD_decay;
    float continousLOD_start;
};

