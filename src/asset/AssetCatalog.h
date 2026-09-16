#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <json/json.hpp>

namespace engine
{
    using GUID = uint64_t; // temporary alias

    /// @brief Maps asset path strings to GUIDs
    class AssetCatalog
    {
    public:
        /// @brief Scans the provided directory recursively for .meta files to populate its internal map of GUIDs.
        /// @param p_directory The directory to be scanned.
        void ScanDirectory(const std::filesystem::path& p_directory);

        /// @brief Retrieves the globally unique identifier for an asset.
        /// @param p_path The path to the asset.
        /// @return The globally unique identifier.
        GUID GetGUID(const std::string& p_path);
        
    private:
        std::unordered_map<std::string, GUID> m_pathToGuid; // Maps paths to GUIDs
    };
}
