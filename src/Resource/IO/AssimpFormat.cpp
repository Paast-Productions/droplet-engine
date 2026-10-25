#include "resource/io/AssimpFormat.hpp"

#include "resource/IResource.hpp"
#include "resource/meta/MetaUtils.hpp"

#include <vector>
#include <string>
#include <utility>
#include <print>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <json/json.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using json = nlohmann::json;

namespace Droplet::IO::AssimpFormat
{
    static constexpr double C_EPSILON = 0.00001;
    
    /// @brief Helper function that constructs a vertex buffer.
    /// @param p_meshData The imported mesh object.
    /// @param p_vertexByteSize The vertex byte size calculated in CreateVertexLayout()
    /// @return A vector of the vertex buffer per byte.
    [[nodiscard]] static std::vector<std::byte> BuildVertexData(const aiScene *&p_meshData, const std::size_t &p_vertexByteSize)
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
    
    
	std::unique_ptr<MeshResource> LoadMesh(const std::filesystem::path &p_assetPath, const json &p_loadSettings)
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

	std::unique_ptr<SkinnedMeshResource> LoadSkinnedMesh(const std::filesystem::path &p_assetPath, const json &p_loadSettings)
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

		std::unordered_map<std::string, int> boneMap{};

		// Old. Replace with node recursive search
		/*for (std::uint32_t i = 0; i < mesh->mNumBones; i++)
		{
			aiBone *bone = mesh->mBones[i];
			aiNode *node = bone->mArmature;

			int parentIndex = -1;
			if (node->mParent != nullptr)
			{
				auto it = boneMap.find(node->mParent->mName.C_Str());
				if (it != boneMap.end())
				{
					parentIndex = it->second;
				}
			}

			aiMatrix4x4 o = node->mTransformation;
			glm::mat4 offsetMat = {
				o.a1, o.a2, o.a3, o.a4,
				o.b1, o.b2, o.b3, o.b4,
				o.c1, o.c2, o.c3, o.c4,
				o.d1, o.d2, o.d3, o.d4,
			};

			boneMap[bone->mName.C_Str()] = skinnedMesh.AddBone(bone->mName.C_Str(), parentIndex, offsetMat);
		}*/

		// Assume there is only one mesh in the scene for now. If there are multiple meshes, we will need to handle that case.
		auto processNode = [&](aiNode *node, int parentIndex, auto &processNodeRef) -> void {
			// Check if the node is a bone
			auto it = std::find_if(mesh->mBones, mesh->mBones + mesh->mNumBones, [&](aiBone *bone) {
				return bone->mName == node->mName;
			});

			int currentIndex = -1;
			if (it != mesh->mBones + mesh->mNumBones)
			{
				aiMatrix4x4 t = node->mTransformation;

				glm::mat4 mat = {
					t.a1, t.a2, t.a3, t.a4,
					t.b1, t.b2, t.b3, t.b4,
					t.c1, t.c2, t.c3, t.c4,
					t.d1, t.d2, t.d3, t.d4,
				};

				currentIndex = skinnedMesh.AddBone(node->mName.C_Str(), parentIndex, mat);
				boneMap[node->mName.C_Str()] = currentIndex;
			}

			for (std::uint32_t i = 0; i < node->mNumChildren; i++)
			{
				processNodeRef(node->mChildren[i], currentIndex != -1 ? currentIndex : parentIndex, processNodeRef);
			}
		};

		processNode(meshData->mRootNode, -1, processNode);

		skinnedMesh.SetMeshData(vertexData, indexData, vertexByteSize, vertexLayout);

		assert(skinnedMesh.ValidateSkinnedVertexLayout() && "Vertex layout is invalid for skinned mesh.");

		skinnedMesh.GenerateBoneBounds();

		// Log Info: Successfully loaded p_meshFile
		std::println("Successfully loaded {}", p_assetPath.generic_string()); // HACK: Temporary log

		s_importer.FreeScene();

		return std::make_unique<SkinnedMeshResource>(skinnedMesh);
	}

	std::unique_ptr<AnimationResource> LoadAnimation(const std::filesystem::path &p_assetPath, const std::string &p_animName, 
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

	std::vector<std::pair<ResourceType, std::string>> ScanAssetResources(const std::filesystem::path &p_assetPath)
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


}

