#include "AssimpLoader.hpp"

#include "asset/Resource.hpp"

#include <print>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <json/json.hpp>

using json = nlohmann::json;

namespace Droplet
{
	bool AssimpLoader::LoadMesh(std::string p_meshFile, const json &p_typeSpecificData, AssetRecord &p_assetRecord)
	{
		p_typeSpecificData;
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), // TODO: Use p_typeSpecificData when importing mesh
			//aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (meshData == nullptr)
		{
			// Log Error: Failed reading mesh file. (m_importer.GetErrorString())
			std::println("{}", m_importer.GetErrorString()); // Temporary log
			return false;
		}

		if (!meshData->HasMeshes())
		{
			// Log Error: Mesh does not contain mesh data.
			return false;
		}

		// TODO: Save mesh data to IResource in p_assetRecord
		MeshResource meshResource;
		std::vector<std::byte> vertexData = BuildVertexData(meshData);
		// meshResource.SetVertexData(meshData->mMeshes[0]->mVertices);
		// meshResource.SetIndexData(meshData->mMeshes[0]->mFaces);
		// meshResource.SetVertexByteSize();
		// meshResource.SetVertexLayout();
		p_assetRecord.resource = std::make_shared<MeshResource>(meshResource);

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_meshFile); // Temporary log

		return true;
	}

	std::vector<std::byte> AssimpLoader::BuildVertexData(const aiScene *&p_meshData)
	{
		std::vector<float> vertices;
		aiMesh *mesh = p_meshData->mMeshes[0];
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			vertices.push_back(mesh->mVertices[i].x);
			vertices.push_back(mesh->mVertices[i].y);
			vertices.push_back(mesh->mVertices[i].z);

			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);

			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}

		const unsigned int C_VERTEX_BYTE_SIZE = 8;
		std::vector<std::byte> vertexData(mesh->mNumVertices * C_VERTEX_BYTE_SIZE * sizeof(float));
		memcpy(vertexData.data(), vertices.data(), mesh->mNumVertices * C_VERTEX_BYTE_SIZE * sizeof(float));

		return vertexData;
	}
}

