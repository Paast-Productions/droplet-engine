#pragma once
#include "resource/types/Texture2DResource.hpp"
#include "resource/types/Texture3DResource.hpp"

#include <json/json.hpp>
#include <memory>
#include <string>
#include <filesystem>

namespace Droplet::GliLoader
{
	/// @brief Loads a texture from the specified path.
	/// @return A unique pointer to the texture file.
	/// @param p_assetPath Path to the texture asset.
	/// @throws std::runtime_error If the texture cannot be found or loaded.
	[[nodiscard]] std::unique_ptr<Texture2DResource> LoadTexture2D(const std::filesystem::path &p_assetPath, const nlohmann::json &p_loadSettings);
    
    // TODO: Implement
    // [[nodiscard]] std::unique_ptr<Texture3DResource> LoadTexture3D(const std::filesystem::path &p_assetPath); 

    /// @brief Lists the resources in a texture file. This function is just a formality as texture files only contain
    /// one texture.
    /// @param p_assetPath The path to texture asset.
    /// @return A list of all resources found in the texture file.
    [[nodiscard]] std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::filesystem::path &p_assetPath);
}