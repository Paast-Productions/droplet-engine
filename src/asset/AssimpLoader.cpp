#include "AssimpLoader.hpp"

#include "asset/Resource.hpp"

#include <vector>
#include <string>
#include <utility>
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

		// Store the mesh data into p_assetRecord.resource
		MeshResource meshResource;
		std::size_t vertexByteSize = 0; // Size dependent on vertex layout
		std::vector<MeshResource::VertexAttribute> vertexLayout = CreateVertexLayout(vertexByteSize);
		std::vector<std::byte> vertexData = BuildVertexData(meshData, vertexByteSize);
		std::vector<std::uint32_t> indexData = BuildIndexData(meshData);
		meshResource.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);
		p_assetRecord.resource = std::make_shared<MeshResource>(meshResource);

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_meshFile); // Temporary log

		return true;
	}

	std::vector<std::pair<ResourceType, std::string>> AssimpLoader::ListAssetResources(const std::string p_meshFile)
	{
		std::vector<std::pair<ResourceType, std::string>> resourceList;
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), 0);

		if (meshData == nullptr)
		{
			// Log Error: Failed reading mesh file. (m_importer.GetErrorString())
			std::println("{}", m_importer.GetErrorString()); // Temporary log
			return resourceList; // Return empty list
		}

		if (meshData->HasMeshes())
		{
			ResourceType rType = ResourceType::Mesh;
			if (meshData->HasSkeletons())
			{
				rType = ResourceType::SkinnedMesh;
			}

			for (unsigned int i = 0; i < meshData->mNumMeshes; i++)
			{
				resourceList.push_back(std::make_pair(rType, meshData->mMeshes[i]->mName.C_Str()));
			}

			if (rType == ResourceType::SkinnedMesh && meshData->HasAnimations())
			{
				for (unsigned int i = 0; i < meshData->mNumAnimations; i++)
				{
					resourceList.push_back(std::make_pair(ResourceType::Animation, meshData->mAnimations[i]->mName.C_Str()));
				}
			}
		}

		return resourceList;
	}

	std::vector<std::byte> AssimpLoader::BuildVertexData(const aiScene *&p_meshData, const std::size_t p_vertexByteSize)
	{
		std::vector<float> vertices;
		aiMesh *mesh = p_meshData->mMeshes[0];
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			// Manually add the vertex data based of the defined vertex layout
			vertices.push_back(mesh->mVertices[i].x);
			vertices.push_back(mesh->mVertices[i].y);
			vertices.push_back(mesh->mVertices[i].z);

			vertices.push_back(mesh->mNormals[i].x);
			vertices.push_back(mesh->mNormals[i].y);
			vertices.push_back(mesh->mNormals[i].z);

			vertices.push_back(mesh->mTextureCoords[0][i].x);
			vertices.push_back(mesh->mTextureCoords[0][i].y);
		}

		std::vector<std::byte> vertexData(mesh->mNumVertices * p_vertexByteSize);
		memcpy(vertexData.data(), vertices.data(), mesh->mNumVertices * p_vertexByteSize);

		return vertexData;
	}

	std::vector<unsigned int> AssimpLoader::BuildIndexData(const aiScene *&p_meshData)
	{
		std::vector<unsigned int> indices;
		aiMesh *mesh = p_meshData->mMeshes[0];

		// Indices are stored in each face
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			// Iterate through all indices of each face and push them to the vector
			for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				indices.push_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		return indices;
	}

	std::vector<MeshResource::VertexAttribute> AssimpLoader::CreateVertexLayout(std::size_t &p_vertexByteSize)
	{
		std::vector<MeshResource::VertexAttribute> vertexLayout = 
		{
			std::make_pair("POSITION", 3 * sizeof(float)),
			std::make_pair("NORMAL",   3 * sizeof(float)),
			std::make_pair("UV",       2 * sizeof(float))
		};

		// Calculates the total size of a vertex based of what's set in vertexLayout
		for (MeshResource::VertexAttribute v : vertexLayout)
		{
			p_vertexByteSize += v.second;
		}

		return vertexLayout;
	}
}

