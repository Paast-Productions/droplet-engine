#pragma once

#include "asset/GUID.hpp"
#include "asset/meta/MetaData.hpp"

#include <string>
#include <unordered_map>
#include <filesystem>

namespace Droplet
{
    /// @brief Maps asset path strings to GUIDs
    class ResourceCatalog
    {
    public:
        /// @brief Scans the provided directory recursively for .meta files to populate its internal map of GUIDs.
        /// @param p_directory The directory to be scanned.
        /// @return True if directory exists and was scanned, otherwise false.
        bool ScanDirectory(const std::filesystem::path &p_directory);

        /// @brief Retrieves the globally unique identifier for an asset.
        /// @param p_path The path to the asset.
        /// @return The globally unique identifier.
        bool GetResourceMetaData(GUID p_guid, ResourceMetaData &p_resourceData);
        
        void RegisterResourceData(const ResourceMetaData &p_resourceData);
        
    private:
        std::unordered_map<GUID, ResourceMetaData> m_metaDataMap;
    };
}
