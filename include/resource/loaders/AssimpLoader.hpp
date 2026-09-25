#pragma once

#include "resource/ResourceManager.hpp"
#include "resource/types/MeshResource.hpp"
#include "resource/types/SkinnedMeshResource.hpp"
#include "resource/types/AnimationResource.hpp"

#include <vector>
#include <string>
#include <utility>

namespace Droplet::AssimpLoader
{
	/// @brief Loads a mesh from an asset file.
	/// @param p_assetPath Path to the asset file storing the mesh.
	/// @param p_typeSpecificData Mesh specific import settings.
	/// @return A unique pointer with the stored mesh data.
	[[nodiscard]] std::unique_ptr<MeshResource> LoadMesh(const std::filesystem::path &p_assetPath, const nlohmann::json &p_typeSpecificData);

	/// @brief Loads a skinned mesh from an asset file.
	/// @param p_assetPath Path to the asset file storing the skinned mesh.
	/// @param p_typeSpecificData Mesh specific import settings.
	/// @return A unique pointer with the stored skinned mesh data.
	[[nodiscard]] std::unique_ptr<SkinnedMeshResource> LoadSkinnedMesh(const std::filesystem::path &p_assetPath, const nlohmann::json &p_typeSpecificData);

	/// @brief Loads an animation from an asset file.
	/// @param p_meshFile Path to the asset file storing the animation.
	/// @param p_animName The name given to the animation within the asset file.
	/// @param p_typeSpecificData Mesh specific import settings.
	/// @return A unique pointer with the stored animation data.
	[[nodiscard]] std::unique_ptr<AnimationResource> LoadAnimation(const std::filesystem::path &p_assetPath, const std::string &p_animName,
		const nlohmann::json &p_typeSpecificData);

	/// @brief Lists all resources in a model file.
	/// @param p_assetPath File path of the model file.
	/// @return A vector of resource types and their names.
	[[nodiscard]] std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::filesystem::path &p_assetPath);
}

