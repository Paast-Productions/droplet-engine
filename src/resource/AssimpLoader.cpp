#include "AssimpLoader.hpp"

#include "resource/IResource.hpp"

#include <vector>
#include <string>
#include <utility>
#include <print>
#include <algorithm>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <json/json.hpp>

using json = nlohmann::json;

namespace Droplet
{
	std::unique_ptr<MeshResource> AssimpLoader::LoadMesh(const std::string &p_meshFile, const json &p_typeSpecificData)
	{
		p_typeSpecificData; // HACK
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), // TODO: Use p_typeSpecificData when importing mesh
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (meshData == nullptr)
		{
			m_importer.FreeScene();
			throw std::runtime_error(m_importer.GetErrorString());
		}

		if (!meshData->HasMeshes())
		{
			m_importer.FreeScene();
			throw std::runtime_error("Mesh does not contain mesh data.");
		}

		// Store the mesh data into p_assetRecord.resource
		std::size_t vertexByteSize = 0; // Size dependent on vertex layout
		std::vector<MeshResource::VertexAttribute> vertexLayout = CreateVertexLayout(vertexByteSize);
		std::vector<std::byte> vertexData = BuildVertexData(meshData, vertexByteSize);
		std::vector<std::uint32_t> indexData = BuildIndexData(meshData);

		MeshResource mesh{};
		mesh.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_meshFile); // Temporary log

		m_importer.FreeScene();

		return std::make_unique<MeshResource>(mesh);
	}

	std::unique_ptr<SkinnedMeshResource> AssimpLoader::LoadSkinnedMesh(const std::string &p_meshFile, const json &p_typeSpecificData)
	{
		p_typeSpecificData;	// HACK
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), // TODO: Use p_typeSpecificData when importing mesh
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

		if (meshData == nullptr)
		{
			m_importer.FreeScene();
			throw std::runtime_error(m_importer.GetErrorString());
		}

		if (!meshData->HasMeshes())
		{
			m_importer.FreeScene();
			throw std::runtime_error("Mesh does not contain mesh data.");
		}

		if (!meshData->HasAnimations())
		{
			m_importer.FreeScene();
			throw std::runtime_error("Mesh does not contain animation data.");
		}

		std::size_t vertexByteSize = 0; // Size dependent on vertex layout
		std::vector<MeshResource::VertexAttribute> vertexLayout = CreateVertexLayout(vertexByteSize);
		std::vector<std::byte> vertexData = BuildVertexData(meshData, vertexByteSize);
		std::vector<std::uint32_t> indexData = BuildIndexData(meshData);

		SkinnedMeshResource skinnedMesh{};
		int parentIndex = -1;
		aiMesh *mesh = meshData->mMeshes[0];
		for (std::uint32_t i = 0; i < mesh->mNumBones; i++)
		{
			aiBone *bone = mesh->mBones[i];
			aiMatrix4x4 o = bone->mOffsetMatrix;
			glm::mat4 offsetMat =
			{
				o.a1, o.a2, o.a3, o.a4,
				o.b1, o.b2, o.b3, o.b4,
				o.c1, o.c2, o.c3, o.c4,
				o.d1, o.d2, o.d3, o.d4,
			};
			parentIndex = skinnedMesh.AddBone(bone->mName.C_Str(), parentIndex, offsetMat);
		}

		skinnedMesh.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_meshFile); // Temporary log

		m_importer.FreeScene();

		return std::make_unique<SkinnedMeshResource>(skinnedMesh);
	}

	std::unique_ptr<AnimationResource> AssimpLoader::LoadAnimation(const std::string &p_meshFile, const std::string &p_animName,
		const nlohmann::json &p_typeSpecificData)
	{
		p_typeSpecificData; // HACK
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), // TODO: Use p_typeSpecificData when importing mesh
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_PopulateArmatureData |
			aiProcess_SortByPType);

		if (meshData == nullptr)
		{
			m_importer.FreeScene();
			throw std::runtime_error(m_importer.GetErrorString());
		}

		if (!meshData->HasAnimations())
		{
			m_importer.FreeScene();
			throw std::runtime_error("Mesh does not contain animation data.");
		}

		std::vector<AnimationResource::AnimKeyframe> animKeyframes{};
		for (std::size_t i = 0; i < meshData->mNumAnimations; i++)
		{
			aiAnimation *anim = meshData->mAnimations[i];
			std::string animName = anim->mName.C_Str();
			if (animName != p_animName)
			{
				continue;
			}

			// Find all unique keyframe times
			for (std::size_t j = 0; j < anim->mNumChannels; j++) // Channels are bones
			{
				aiNodeAnim *nodeAnim = anim->mChannels[j]; // Current bone
				AnimationResource::AnimKeyframe animKeyframe{};
				for (std::uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++)
				{
					animKeyframe.time = static_cast<float>(nodeAnim->mPositionKeys[k].mTime / anim->mTicksPerSecond);
					animKeyframes.push_back(animKeyframe);
				}

				for (std::uint32_t k = 0; k < nodeAnim->mNumRotationKeys; k++)
				{
					animKeyframe.time = static_cast<float>(nodeAnim->mRotationKeys[k].mTime / anim->mTicksPerSecond);
					animKeyframes.push_back(animKeyframe);
				}

				for (std::uint32_t k = 0; k < nodeAnim->mNumScalingKeys; k++)
				{
					animKeyframe.time = static_cast<float>(nodeAnim->mScalingKeys[k].mTime / anim->mTicksPerSecond);
					animKeyframes.push_back(animKeyframe);
				}
			}

			// Sorts the vector from lowest to highest based of AnimationResource::AnimKeyframe::time
			std::sort(animKeyframes.begin(), animKeyframes.end(),
				[](const AnimationResource::AnimKeyframe &a, const AnimationResource::AnimKeyframe &b) {
					return a.time < b.time;
				});

			// Erases duplicate elements based of AnimationResource::AnimKeyframe::time
			animKeyframes.erase(std::unique(animKeyframes.begin(), animKeyframes.end(),
				[](const AnimationResource::AnimKeyframe &a, const AnimationResource::AnimKeyframe &b) {
					return a.time == b.time;
				}),
				animKeyframes.end());
			
			std::uint32_t posIndex = 0, rotIndex = 0, scaIndex = 0; // Translation indices
			for (AnimationResource::AnimKeyframe &a : animKeyframes) // Store each bone keyframe
			{
				double t = a.time;
				for (std::size_t j = 0; j < meshData->mAnimations[i]->mNumChannels; j++) // Channels are bones
				{
					aiNodeAnim *nodeAnim = anim->mChannels[j]; // Current bone
					AnimationResource::BoneKeyframe boneKeyframe{};
					boneKeyframe.boneName = nodeAnim->mNodeName.C_Str();

					if (t == nodeAnim->mPositionKeys[posIndex].mTime / anim->mTicksPerSecond) // Position
					{
						aiVectorKey posKey = nodeAnim->mPositionKeys[posIndex];
						aiVector3D pos = posKey.mValue;
						boneKeyframe.pos = { pos.x, pos.y, pos.z };
						posIndex++;
					}

					if (t == nodeAnim->mRotationKeys[rotIndex].mTime / anim->mTicksPerSecond) // Rotation
					{
						aiQuatKey rotKey = nodeAnim->mRotationKeys[posIndex];
						aiQuaternion rot = rotKey.mValue;
						boneKeyframe.rot = { rot.w, rot.x, rot.y, rot.z };
						rotIndex++;
					}

					if (t == nodeAnim->mScalingKeys[scaIndex].mTime / anim->mTicksPerSecond) // Scale
					{
						aiVectorKey scaKey = nodeAnim->mScalingKeys[scaIndex];
						aiVector3D sca = scaKey.mValue;
						boneKeyframe.scale = { sca.x, sca.y, sca.z };
						scaIndex++;
					}

					a.boneKeyframes.push_back(boneKeyframe);
				}
			}
		}

		AnimationResource animation{};
		animation.SetName(p_animName);
		animation.SetKeyframes(animKeyframes);

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_meshFile); // Temporary log

		m_importer.FreeScene();

		return std::make_unique<AnimationResource>(animation);
	}

	std::vector<std::pair<ResourceType, std::string>> AssimpLoader::ListAssetResources(const std::string &p_meshFile)
	{
		std::vector<std::pair<ResourceType, std::string>> resourceList{};
		const aiScene *meshData = m_importer.ReadFile(p_meshFile.c_str(), 0);

		if (meshData == nullptr)
		{
			m_importer.FreeScene();
			throw std::runtime_error(m_importer.GetErrorString());
			return resourceList; // Return empty list
		}

		if (meshData->HasMeshes())
		{
			ResourceType rType = ResourceType::Mesh;
			if (meshData->HasSkeletons())
			{
				rType = ResourceType::SkinnedMesh;
			}

			for (std::uint32_t i = 0; i < meshData->mNumMeshes; i++)
			{
				resourceList.emplace_back(rType, meshData->mMeshes[i]->mName.C_Str());
			}

			if (rType == ResourceType::SkinnedMesh && meshData->HasAnimations())
			{
				for (std::uint32_t i = 0; i < meshData->mNumAnimations; i++)
				{
					resourceList.emplace_back(ResourceType::Animation, meshData->mAnimations[i]->mName.C_Str());
				}
			}
		}

		m_importer.FreeScene();

		return resourceList;
	}

	std::vector<std::byte> AssimpLoader::BuildVertexData(const aiScene *&p_meshData, const std::size_t &p_vertexByteSize)
	{
		aiMesh *mesh = p_meshData->mMeshes[0];
		std::vector<float> vertices{};
		vertices.reserve(mesh->mNumVertices * p_vertexByteSize / sizeof(float));
		for (std::uint32_t i = 0; i < mesh->mNumVertices; i++)
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
		std::memcpy(vertexData.data(), vertices.data(), mesh->mNumVertices * p_vertexByteSize);

		return vertexData;
	}

	std::vector<std::uint32_t> AssimpLoader::BuildIndexData(const aiScene *&p_meshData)
	{
		aiMesh *mesh = p_meshData->mMeshes[0];
		std::vector<std::uint32_t> indices{};
		indices.reserve(static_cast<std::size_t>(mesh->mNumFaces * 3)); // Assuming the mesh is triangulated
		// Indices are stored in each face
		for (std::uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			// Iterate through all indices of each face and push them to the vector
			for (std::uint32_t j = 0; j < mesh->mFaces[i].mNumIndices; j++)
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

