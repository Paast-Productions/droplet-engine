#pragma once

#include "resource/GuidUtils.hpp"
#include "resource/meta/MetaData.hpp"

#include <string>
#include <unordered_map>
#include <filesystem>

namespace Droplet
{
    /// @brief Maps asset path strings to GUIDs
    class ResourceRegistry
    {
    public:
        /// @brief Scans the provided directory recursively for .meta files to populate its internal map of GUIDs.
        /// @param p_directory The directory to be scanned.
        /// @return True if directory exists and was scanned, otherwise false.
        bool ScanDirectory(const std::filesystem::path &p_directory);

        /// @brief Meta entry data for a specific resource.
        /// @param p_guid The GUID of the resource.
        /// @param p_metaEntry The entry data for that resource in the metafile.
        /// @return The globally unique identifier.
        bool GetResourceMetaData(GUID p_guid, MetaEntry &p_metaEntry);

        /// @brief Fetches all metadata bodies associated with a specific asset.
        /// @param p_assetPath The path to the asset (not its .meta file)
        /// @param p_metaData The list of resources in that asset. 
        /// @return True if the asset is registered and contains resources, otherwise false.
        bool GetCachedMetaDataForAsset(const std::string &p_assetPath, std::vector<MetaEntry> &p_metaData);

        /// @brief Retrieves a list of all resources in the registry, optionally filtered by type.
        /// @param p_type The resource type to filter by. Default is None (this returns all resource types).
        /// @return A vector of pointers to the registered metadata entries.
        std::vector<const MetaEntry *> GetEntries(ResourceType p_type = ResourceType::None) const;

        /// @brief Registers and updates a resource in the registry.
        /// @param p_assetPath The path to the asset file.
        /// @param p_metaEntry The meta entry to be registered.
        void RegisterMetaEntry(const std::string &p_assetPath, const MetaEntry &p_metaEntry);
        
    private:
        std::unordered_map<GUID, MetaEntry> m_guidToEntryMap;
        std::unordered_map<std::string, std::vector<GUID>> m_assetToResourcesMap;
    };
}
