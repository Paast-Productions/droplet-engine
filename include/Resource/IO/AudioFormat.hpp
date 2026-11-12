#pragma once

#include <vector>
#include <filesystem>
#include <utility>

namespace Droplet::IO::AudioFormat
{
    /// @brief Lists all resources in an audio file.
    /// 
    /// @param p_assetPath File path of the audio file.
    /// @return A vector of resource types and their names.
    /// @throw std::runtime_error If it failed to find the file.
    [[nodiscard]] std::vector<std::pair<StaticResourceType, std::string>> ScanAssetResources(const std::filesystem::path &p_assetPath);
}