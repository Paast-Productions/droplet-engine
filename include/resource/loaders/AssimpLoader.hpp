#pragma once

#include "resource/types/MeshResource.hpp"
#include "resource/types/SkinnedMeshResource.hpp"
#include "resource/types/AnimationResource.hpp"

#include <json/json.hpp>

#include <vector>
#include <string>
#include <utility>

namespace Droplet::AssimpLoader
{
	/// @brief Loads a mesh from an asset file.
	/// @param p_assetPath Path to the asset file storing the mesh.
	/// @param p_loadSettings Mesh specific import settings.
	/// @return A unique pointer with the stored mesh data.
	/// @throw std::runtime_error If Assimp failed to read the asset file or if the asset does not contain any meshes.
	[[nodiscard]] std::unique_ptr<MeshResource> LoadMesh(const std::filesystem::path &p_assetPath, const nlohmann::json &p_loadSettings);

	/// @brief Loads a skinned mesh from an asset file.
	/// @param p_assetPath Path to the asset file storing the skinned mesh.
	/// @param p_loadSettings Mesh specific import settings.
	/// @return A unique pointer with the stored skinned mesh data.
	/// @throw std::runtime_error If Assimp failed to read the asset file or if the asset does not contain any meshes or animations.
	[[nodiscard]] std::unique_ptr<SkinnedMeshResource> LoadSkinnedMesh(const std::filesystem::path &p_assetPath, const nlohmann::json &p_loadSettings);

	/// @brief Loads an animation from an asset file.
	/// @param p_assetPath Path to the asset file storing the animation.
	/// @param p_loadSettings Mesh specific import settings.
	/// @return A unique pointer with the stored animation data.
	/// @throw std::runtime_error If Assimp failed to read the asset file or if the asset does not contain any animations.
	[[nodiscard]] std::unique_ptr<AnimationResource> LoadAnimation(const std::filesystem::path &p_assetPath, const nlohmann::json &p_loadSettings);

	/// @brief Lists all resources in a model file.
	/// @param p_assetPath File path of the model file.
	/// @return A vector of resource types and their names.
    /// @throw std::runtime_error If Assimp failed to read the asset file.
	[[nodiscard]] std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::filesystem::path &p_assetPath);
}

