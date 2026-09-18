#include "AssimpLoader.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <json/json.hpp>

using json = nlohmann::json;

namespace Droplet
{
	bool AssimpLoader::LoadMesh(std::string p_meshFile, const json &p_typeSpecificData, AssetRecord &p_assetRecord)
	{
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), // TODO: Use p_typeSpecificData when importing mesh
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (meshData == nullptr)
		{
			// Log Error: Failed reading mesh file. (m_importer.GetErrorString())
			return false;
		}

		if (!meshData->HasMeshes())
		{
			// Log Error: Mesh does not contain mesh data.
			return false;
		}

		// TODO: Save mesh data to IResource in p_assetRecord

		// Log Info: Successfully loaded p_meshFile

		return true;
	}
}

