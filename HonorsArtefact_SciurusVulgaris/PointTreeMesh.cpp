#include "PCH.h"
#include "PointTreeMesh.h"
#include "json.hpp"


void PointTreeMesh::LoadFromTreeFile(std::string path)
{
	std::ifstream file(path);
	nlohmann::json data = nlohmann::json::parse(file);
	file.close();

	std::string modelPath = path.substr(0, path.length() - 5) + "_MODELDATA.fbx";
	LoadFromFile(modelPath);

	levelOfDetailType = static_cast<LODType>(data["levelOfDetailType"]);
	
	switch (levelOfDetailType) {
	case LODType::RANDOM_LEVELS:
		randomLevelsLODPointCount.clear();
		for (auto& item : data["randomLevelsPointCount"]) {
			randomLevelsLODPointCount.push_back(item);
		}
		break;
	}
}

void PointTreeMesh::SaveToTreeFile(std::string path)
{
	std::string modelPath = path.substr(0, path.length() - 5) + "_MODELDATA.fbx";
	SaveToFile(modelPath);
	
	nlohmann::json output;
	switch (levelOfDetailType) {
	case LODType::RANDOM_LEVELS:
		output["levelOfDetailType"] = 0;
		output["randomLevelsPointCount"] = nlohmann::json::array();
		for (auto& item : randomLevelsLODPointCount) {
			output["randomLevelsPointCount"].push_back(item);
		}
		break;
	}

	std::ofstream file(path);
	file << output;
	file.close();
}
