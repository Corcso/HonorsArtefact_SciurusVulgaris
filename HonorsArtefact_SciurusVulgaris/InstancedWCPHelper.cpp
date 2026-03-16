#include "PCH.h"
#include "InstancedWCPHelper.h"
#include <random>

void InstancedWCPHelper::LoadFromFile(std::string path)
{
    // Read from the text file
    std::ifstream OBJFile(path);

    std::string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (std::getline(OBJFile, line)) {
        if (line.length() < 3) continue;
        if (line[0] == 'v') {
            std::string thisLinePoints[3];
            int currentPointIndex = 0;
            std::string currentPoint = "";

            for (int c = 2; c < line.length(); ++c) {
                if (line[c] == ' ') {
                    thisLinePoints[currentPointIndex] = currentPoint;
                    currentPoint = "";
                    currentPointIndex++;
                }
                else {
                    currentPoint += line[c];
                }
            }
            thisLinePoints[currentPointIndex] = currentPoint;

            matrices.push_back({
                   HMM_Translate(HMM_V3(std::stof(thisLinePoints[0]), std::stof(thisLinePoints[1]), std::stof(thisLinePoints[2])) * 100), HMM_M4D(1), HMM_M4D(1) });
        }
    }

    // Close the file
    OBJFile.close();
}
void InstancedWCPHelper::SetViewAndProjection(HMM_Mat4 view, HMM_Mat4 projection)
{
	for (auto& WCP : matrices) {
		WCP.camera = view;
		WCP.projection = projection;
	}
}

void InstancedWCPHelper::ApplyAlternateTransform(HMM_Mat4 transform)
{
    for (auto& WCP : matrices) {
        WCP.world = WCP.world * transform;
    }
}

uint64_t InstancedWCPHelper::ApplyRandomRotation(uint64_t seed)
{
    std::random_device rd;
    uint64_t chosenSeed = seed == 0 ? rd() : seed;
    std::mt19937 chooseRand(chosenSeed);
    for (auto& WCP : matrices) {
        WCP.world = WCP.world * HMM_Rotate_LH(chooseRand(), HMM_V3(0, 1, 0));
    }

    return chosenSeed;
}

void InstancedWorldMatrixHelper::LoadFromFile(std::string path)
{
    // Read from the text file
    std::ifstream OBJFile(path);

    std::string line;

    // Use a while loop together with the getline() function to read the file line by line
    while (std::getline(OBJFile, line)) {
        if (line.length() < 3) continue;
        if (line[0] == 'v') {
            std::string thisLinePoints[3];
            int currentPointIndex = 0;
            std::string currentPoint = "";

            for (int c = 2; c < line.length(); ++c) {
                if (line[c] == ' ') {
                    thisLinePoints[currentPointIndex] = currentPoint;
                    currentPoint = "";
                    currentPointIndex++;
                }
                else {
                    currentPoint += line[c];
                }
            }
            thisLinePoints[currentPointIndex] = currentPoint;

            matrices.push_back(HMM_Translate(HMM_V3(std::stof(thisLinePoints[0]), std::stof(thisLinePoints[1]), std::stof(thisLinePoints[2])) * 100));
        }
    }

    // Close the file
    OBJFile.close();
}

void InstancedWorldMatrixHelper::ApplyAlternateTransform(HMM_Mat4 transform)
{
    for (auto& matrix : matrices) {
        matrix = matrix * transform;
    }
}

uint64_t InstancedWorldMatrixHelper::ApplyRandomRotation(uint64_t seed)
{
    std::random_device rd;
    uint64_t chosenSeed = seed == 0 ? rd() : seed;
    std::mt19937 chooseRand(chosenSeed);
    for (auto& matrix : matrices) {
        matrix = matrix * HMM_Rotate_LH(chooseRand(), HMM_V3(0, 1, 0));
    }

    return chosenSeed;
}
