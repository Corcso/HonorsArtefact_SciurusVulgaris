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
	case LODType::CONTINUOUS:
		continousLOD_decay = data["decay"];
		continousLOD_shallowness = data["shallowness"];
		continousLOD_start = data["start"];
		break;
	}
}

void PointTreeMesh::SaveToTreeFile(std::string path)
{
	
	
	nlohmann::json output;
	switch (levelOfDetailType) {
	case LODType::RANDOM_LEVELS:
		output["levelOfDetailType"] = 0;
		output["randomLevelsPointCount"] = nlohmann::json::array();
		for (auto& item : randomLevelsLODPointCount) {
			output["randomLevelsPointCount"].push_back(item);
		}

		// Truncate model if needed
		if (randomLevelsLODPointCount.size() > 0 && randomLevelsLODPointCount[0] < points.size()) {
			points.resize(randomLevelsLODPointCount[0]);
		}
		break;
	case LODType::CONTINUOUS:
		output["levelOfDetailType"] = 1;
		output["decay"] = continousLOD_decay;
		output["shallowness"] = continousLOD_shallowness;
		output["start"] = continousLOD_start;

		// Truncate model if needed
		if (randomLevelsLODPointCount.size() > 0 && continousLOD_start < points.size()) {
			points.resize(randomLevelsLODPointCount[0]);
		}
		break;
	}

	std::ofstream file(path);
	file << output;
	file.close();

	std::string modelPath = path.substr(0, path.length() - 5) + "_MODELDATA.fbx";
	SaveToFile(modelPath);
}
