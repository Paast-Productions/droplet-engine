#include "resource/io/AssimpFormat.hpp"

#include "resource/IResource.hpp"
#include "resource/meta/MetaUtils.hpp"

#include <vector>
#include <string>
#include <utility>
#include <print>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <json/json.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using json = nlohmann::json;

using namespace Droplet;
using namespace Droplet::IO::AssimpFormat;

static constexpr double C_EPSILON = 0.00001;
    
/// @brief Helper function that constructs a vertex buffer.
/// @param p_meshData The imported mesh object.
/// @param p_vertexByteSize The vertex byte size calculated in CreateVertexLayout()
/// @return A vector of the vertex buffer per byte.
[[nodiscard]] static std::vector<std::byte> BuildVertexData(const aiScene *&p_meshData, const std::size_t &p_vertexByteSize)
{
	aiMesh *mesh = p_meshData->mMeshes[0];
	std::vector<std::byte> vertexData(mesh->mNumVertices * p_vertexByteSize);
	for (std::uint32_t i = 0; i < mesh->mNumVertices; i++)
	{
		// Manually add the vertex data based of the defined vertex layout, one vertex per p_vertexByteSize stride
		const float attributes[8] = {
			mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z,
			mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z,
			mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
		};
		std::memcpy(vertexData.data() + i * p_vertexByteSize, attributes, sizeof(attributes));
	}

	return vertexData;
}

/// @brief Converts an Assimp matrix (row-major, a1..a4 is the first row) to a GLM matrix (column-major, m[column][row]).
[[nodiscard]] static glm::mat4 ConvertMatrix(const aiMatrix4x4 &p_matrix)
{
	glm::mat4 result(1.0f);
	for (int row = 0; row < 4; ++row)
	{
		for (int column = 0; column < 4; ++column)
		{
			result[column][row] = p_matrix[row][column];
		}
	}
	return result;
}

/// @brief Helper function that constructs a index buffer.
/// @param p_meshData The imported mesh object.
/// @return A vector of the index buffer.
[[nodiscard]] static std::vector<std::uint32_t> BuildIndexData(const aiScene *&p_meshData)
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
    
/// @brief Helper function that creates the vertex layout.
/// @param[out] p_vertexByteSize The size of the created vertex layout.
/// @return A vector of the vertex layout.
[[nodiscard]] static std::vector<MeshResource::VertexAttribute> CreateVertexLayout(std::size_t &p_vertexByteSize, bool p_skinned = false)
{
	std::vector<MeshResource::VertexAttribute> vertexLayout = {
		std::make_pair("POSITION", 3 * sizeof(float)),
		std::make_pair("NORMAL",   3 * sizeof(float)),
		std::make_pair("UV",       2 * sizeof(float))
	};

	if (p_skinned)
	{
		vertexLayout.emplace_back("BONE_INDICES", 4 * sizeof(int));
		vertexLayout.emplace_back("BONE_WEIGHTS", 4 * sizeof(float));
	}

	// Calculates the total size of a vertex based of what's set in vertexLayout
	for (MeshResource::VertexAttribute attribute : vertexLayout)
	{
		p_vertexByteSize += attribute.second;
	}

	return vertexLayout;
}
    
    
std::unique_ptr<MeshResource> Droplet::IO::AssimpFormat::LoadMesh(const std::filesystem::path &p_assetPath, const json &p_loadSettings)
{
	ZoneScoped;

	thread_local Assimp::Importer s_importer;
        
    bool generateNormals = p_loadSettings.value(MetaLoadSettings::C_GENERATE_NORMALS.key, MetaLoadSettings::C_GENERATE_NORMALS.defaultValue);
    bool joinVertices    = p_loadSettings.value(MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.key, MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.defaultValue);
    bool triangulate     = p_loadSettings.value(MetaLoadSettings::C_TRIANGULATE.key, MetaLoadSettings::C_TRIANGULATE.defaultValue);
        
    unsigned int flags = aiProcess_SortByPType | aiProcess_ValidateDataStructure;
    if (generateNormals) flags |= aiProcess_ForceGenNormals;
    if (joinVertices)    flags |= aiProcess_JoinIdenticalVertices;
    if (triangulate)     flags |= aiProcess_Triangulate;
        
	const aiScene *meshData = s_importer.ReadFile(p_assetPath.generic_string().c_str(), flags);
	if (meshData == nullptr)
	{
		s_importer.FreeScene();
		throw std::runtime_error(s_importer.GetErrorString());
	}

	if (!meshData->HasMeshes())
	{
		s_importer.FreeScene();
		throw std::runtime_error("Mesh does not contain mesh data.");
	}

	if (meshData->mNumMeshes != 1)
	{
		s_importer.FreeScene();
		throw std::runtime_error(std::format("Mesh file {} contains multiple meshes.", p_assetPath.generic_string()));
	}

	// Store the mesh data into p_assetRecord.resource
	std::size_t vertexByteSize = 0; // Size dependent on vertex layout
	std::vector<MeshResource::VertexAttribute> vertexLayout = CreateVertexLayout(vertexByteSize);
	std::vector<std::byte> vertexData = BuildVertexData(meshData, vertexByteSize);
	std::vector<std::uint32_t> indexData = BuildIndexData(meshData);

	MeshResource mesh{};
	mesh.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);
        
	std::println("Successfully loaded {}", p_assetPath.generic_string()); // TODO: Remove temp log

	s_importer.FreeScene();

	mesh.GenerateBVH();

	return std::make_unique<MeshResource>(mesh);
}

std::unique_ptr<SkinnedMeshResource> Droplet::IO::AssimpFormat::LoadSkinnedMesh(const std::filesystem::path &p_assetPath, const json &p_loadSettings)
{
	ZoneScoped;

    thread_local Assimp::Importer s_importer;
        
    bool generateNormals = p_loadSettings.value(MetaLoadSettings::C_GENERATE_NORMALS.key, MetaLoadSettings::C_GENERATE_NORMALS.defaultValue);
    bool joinVertices    = p_loadSettings.value(MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.key, MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.defaultValue);
    bool triangulate     = p_loadSettings.value(MetaLoadSettings::C_TRIANGULATE.key, MetaLoadSettings::C_TRIANGULATE.defaultValue);
        
    unsigned int flags = aiProcess_SortByPType | aiProcess_ValidateDataStructure | aiProcess_LimitBoneWeights | aiProcess_PopulateArmatureData;
    if (generateNormals) flags |= aiProcess_ForceGenNormals;
    if (joinVertices)    flags |= aiProcess_JoinIdenticalVertices;
    if (triangulate)     flags |= aiProcess_Triangulate;
        
	const aiScene *meshData = s_importer.ReadFile(p_assetPath.generic_string().c_str(), flags);
	if (meshData == nullptr)
	{
		s_importer.FreeScene();
		throw std::runtime_error(s_importer.GetErrorString());
	}

	if (!meshData->HasMeshes())
	{
		s_importer.FreeScene();
		throw std::runtime_error("Mesh does not contain mesh data.");
	}

	if (!meshData->mMeshes[0]->HasBones())
	{
		s_importer.FreeScene();
		throw std::runtime_error("Mesh does not contain skeleton data.");
	}

	std::size_t vertexByteSize = 0; // Size dependent on vertex layout
	std::vector<MeshResource::VertexAttribute> vertexLayout = CreateVertexLayout(vertexByteSize, true);
	std::vector<std::byte> vertexData = BuildVertexData(meshData, vertexByteSize);
	std::vector<std::uint32_t> indexData = BuildIndexData(meshData);

	SkinnedMeshResource skinnedMesh{};
	aiMesh *mesh = meshData->mMeshes[0];

	// Coordinate-space convention (see SkinnedMeshResource):
	//  - Vertices are stored exactly as Assimp provides them (the mesh's own space, no node transform baked in).
	//  - offsetMat is Assimp's mOffsetMatrix: mesh space -> bone space at bind pose.
	//  - The bind pose of a bone in mesh space is inverse(offsetMat).
	//  - transformMat is the bone's bind transform relative to its parent bone, accumulated over any
	//    non-bone intermediary nodes. For root bones it is relative to the scene root.
	std::unordered_map<std::string, int> boneMap{};

	// Add all bones in the order of the mBones array. This ensures that the bone indices are consistent with the vertex bone indices.
	for (std::uint32_t i = 0; i < mesh->mNumBones; i++)
	{
		aiBone *bone = mesh->mBones[i];
		boneMap[bone->mName.C_Str()] = skinnedMesh.AddBone(bone->mName.C_Str());
	}

	std::size_t boneIndicesOffset = 0, boneWeightsOffset = 0, attributeOffset = 0;
	for (const auto &attribute : vertexLayout)
	{
		if (attribute.first == "BONE_INDICES") boneIndicesOffset = attributeOffset;
		if (attribute.first == "BONE_WEIGHTS") boneWeightsOffset = attributeOffset;
		attributeOffset += attribute.second;
	}

	// Fill the per-vertex bone indices and weights
	{
		const std::size_t maxInfluences = 4;
		std::vector<std::size_t> influenceCount(mesh->mNumVertices, 0);
		for (std::uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
		{
			const aiBone *bone = mesh->mBones[boneIndex];
			for (std::uint32_t w = 0; w < bone->mNumWeights; w++)
			{
				const aiVertexWeight &weight = bone->mWeights[w];
				if (weight.mVertexId >= mesh->mNumVertices || weight.mWeight <= 0.0f || influenceCount[weight.mVertexId] >= maxInfluences)
				{
					continue;
				}

				std::byte *vertexPtr = vertexData.data() + static_cast<std::size_t>(weight.mVertexId) * vertexByteSize;
				const std::size_t slot = influenceCount[weight.mVertexId]++;
				const int index = boneMap[bone->mName.C_Str()];
				std::memcpy(vertexPtr + boneIndicesOffset + slot * sizeof(int), &index, sizeof(int));
				std::memcpy(vertexPtr + boneWeightsOffset + slot * sizeof(float), &weight.mWeight, sizeof(float));
			}
		}
	}

	// Resolve the real parent bones from the scene node hierarchy.
	for (std::uint32_t i = 0; i < mesh->mNumBones; i++)
	{
		const aiBone *bone = mesh->mBones[i];
		const glm::mat4 offset = ConvertMatrix(bone->mOffsetMatrix);
		const int thisIndex = boneMap[bone->mName.C_Str()];

		const aiNode *node = meshData->mRootNode ? meshData->mRootNode->FindNode(bone->mName) : nullptr;

		int parentIndex = -1;
		glm::mat4 transform(1.0f);

		if (node != nullptr)
		{
			// Walk up to the closest ancestor that is a bone of this mesh, accumulating non-bone intermediary transforms.
			transform = ConvertMatrix(node->mTransformation);
			for (const aiNode *ancestor = node->mParent; ancestor != nullptr; ancestor = ancestor->mParent)
			{
				const auto parentIt = boneMap.find(ancestor->mName.C_Str());
				if (parentIt != boneMap.end() && parentIt->second != thisIndex)
				{
					parentIndex = parentIt->second;
					break;
				}
				transform = ConvertMatrix(ancestor->mTransformation) * transform;
			}
		}
		else
		{
			// Node is missing from the scene: derive the bind transform from the offset matrices instead (roots only).
			const float det = glm::determinant(offset);
			if (std::isfinite(det) && std::abs(det) > 1e-12f)
			{
				transform = glm::inverse(offset);
			}
		}

		skinnedMesh.UpdateBone(thisIndex, parentIndex, transform, offset);
	}

	skinnedMesh.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);

	assert(skinnedMesh.ValidateSkinnedVertexLayout() && "Vertex layout is invalid for skinned mesh.");

	skinnedMesh.GenerateBoneBounds();

	// Log Info: Successfully loaded p_meshFile
	std::println("Successfully loaded {}", p_assetPath.generic_string()); // HACK: Temporary log

	s_importer.FreeScene();

	return std::make_unique<SkinnedMeshResource>(skinnedMesh);
}

std::unique_ptr<AnimationResource> Droplet::IO::AssimpFormat::LoadAnimation(const std::filesystem::path &p_assetPath, const std::string &p_animName,
	const nlohmann::json &p_loadSettings)
{
	ZoneScoped;

    thread_local Assimp::Importer s_importer;
        
    bool generateNormals =  MetaLoadSettings::C_GENERATE_NORMALS.defaultValue;
    bool joinVertices =     MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.defaultValue;
    bool triangulate =      MetaLoadSettings::C_TRIANGULATE.defaultValue;
        
    unsigned int flags = aiProcess_SortByPType | aiProcess_PopulateArmatureData;
    if (generateNormals)    flags |= aiProcess_ForceGenNormals;
    if (joinVertices)       flags |= aiProcess_JoinIdenticalVertices;
    if (triangulate)        flags |= aiProcess_Triangulate;
        
	const aiScene *meshData = s_importer.ReadFile(p_assetPath.generic_string().c_str(), flags);

	if (meshData == nullptr)
	{
		s_importer.FreeScene();
		throw std::runtime_error(s_importer.GetErrorString());
	}

	if (!meshData->HasAnimations())
	{
		s_importer.FreeScene();
		throw std::runtime_error("Mesh does not contain animation data.");
	}

	std::vector<AnimationResource::AnimKeyframe> animKeyframes{};
	for (std::size_t i = 0; i < meshData->mNumAnimations; i++)
	{
		aiAnimation *anim = meshData->mAnimations[i];
		if (p_animName != anim->mName.C_Str())
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
			
		for (AnimationResource::AnimKeyframe &a : animKeyframes)
		{
			const double t = a.time;

			for (std::size_t j = 0; j < anim->mNumChannels; ++j)
			{
				aiNodeAnim *nodeAnim = anim->mChannels[j];

				AnimationResource::BoneKeyframe boneKeyframe{};
				boneKeyframe.boneName = nodeAnim->mNodeName.C_Str();

				// Position
				if (nodeAnim->mNumPositionKeys > 0)
				{
					std::uint32_t posIndex = 0;

					while (posIndex + 1 < nodeAnim->mNumPositionKeys &&
						nodeAnim->mPositionKeys[posIndex + 1].mTime / anim->mTicksPerSecond <= t)
					{
						++posIndex;
					}

					const double keyTime = nodeAnim->mPositionKeys[posIndex].mTime / anim->mTicksPerSecond;

					if (std::abs(t - keyTime) < C_EPSILON)
					{
						const auto &pos = nodeAnim->mPositionKeys[posIndex].mValue;
						boneKeyframe.pos = { pos.x, pos.y, pos.z };
					}
				}

				// Rotation
				if (nodeAnim->mNumRotationKeys > 0)
				{
					std::uint32_t rotIndex = 0;

					while (rotIndex + 1 < nodeAnim->mNumRotationKeys &&
						nodeAnim->mRotationKeys[rotIndex + 1].mTime / anim->mTicksPerSecond <= t)
					{
						rotIndex++;
					}

					const double keyTime = nodeAnim->mRotationKeys[rotIndex].mTime / anim->mTicksPerSecond;
					if (std::abs(t - keyTime) < C_EPSILON)
					{
						const auto &rot = nodeAnim->mRotationKeys[rotIndex].mValue;
						boneKeyframe.rot = { rot.w, rot.x, rot.y, rot.z };
					}
				}

				// Scale
				if (nodeAnim->mNumScalingKeys > 0)
				{
					std::uint32_t scaIndex = 0;

					while (scaIndex + 1 < nodeAnim->mNumScalingKeys &&
						nodeAnim->mScalingKeys[scaIndex + 1].mTime / anim->mTicksPerSecond <= t)
					{
						scaIndex++;
					}

					const double keyTime = nodeAnim->mScalingKeys[scaIndex].mTime / anim->mTicksPerSecond;
					if (std::abs(t - keyTime) < C_EPSILON)
					{
						const auto &scale = nodeAnim->mScalingKeys[scaIndex].mValue;
						boneKeyframe.scale = { scale.x, scale.y, scale.z };
					}
				}

				a.boneKeyframes.push_back(boneKeyframe);
			}
		}
	}

	if (animKeyframes.size() == 0)
	{
		throw std::runtime_error(std::format("No animation with name {} was found", p_animName));
	}

	AnimationResource animation{};
	animation.SetName(p_animName);
	animation.SetKeyframes(animKeyframes);

	// Log Info: Successfully loaded p_meshFile
	std::println("Successfully loaded {}", p_assetPath.generic_string()); // Temporary log

	s_importer.FreeScene();

	return std::make_unique<AnimationResource>(animation);
}

std::vector<std::pair<ResourceType, std::string>> Droplet::IO::AssimpFormat::ScanAssetResources(const std::filesystem::path &p_assetPath)
{
	ZoneScoped;

    thread_local Assimp::Importer s_importer;
        
	std::vector<std::pair<ResourceType, std::string>> resourceList{};
	const aiScene *meshData = s_importer.ReadFile(p_assetPath.generic_string().c_str(), 0);

	if (meshData == nullptr)
	{
		s_importer.FreeScene();
		throw std::runtime_error(s_importer.GetErrorString());
	}

	if (meshData->HasMeshes())
	{
		bool hasSkinnedMesh = false;
		for (std::uint32_t i = 0; i < meshData->mNumMeshes; i++)
		{
			const aiMesh *mesh = meshData->mMeshes[i];
			resourceList.emplace_back(ResourceType::Mesh, mesh->mName.C_Str());

			// Meshes with bone data can additionally be loaded as skinned meshes
			if (mesh->HasBones())
			{
				hasSkinnedMesh = true;
				resourceList.emplace_back(ResourceType::SkinnedMesh, mesh->mName.C_Str());
			}
		}

		if (hasSkinnedMesh && meshData->HasAnimations())
		{
			for (std::uint32_t i = 0; i < meshData->mNumAnimations; i++)
			{
				resourceList.emplace_back(ResourceType::Animation, meshData->mAnimations[i]->mName.C_Str());
			}
		}
	}

	s_importer.FreeScene();

	return resourceList;
}
