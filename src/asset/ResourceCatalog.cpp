#include "Asset/ResourceCatalog.hpp"

namespace Droplet
{
    bool ResourceCatalog::ScanDirectory(const std::filesystem::path &p_directory)
    {
        if (!std::filesystem::exists(p_directory))
        {
            return false;        
        }
        
        for (const auto &entry : std::filesystem::recursive_directory_iterator(p_directory))
        {
            if (entry.path().extension() == ".meta")
            {
                // Read json formatted meta file
                // Extract guid
                // Derive asset path from file name
                // Add to m_pathToGuid
            }
        }
        
        return true;
    }

    bool ResourceCatalog::GetResourceMetaData(GUID p_guid, MetaEntry &p_metaEntry)
    {
        auto it = m_guidToDataMap.find(p_guid);
        if (it != m_guidToDataMap.end())
        {
            p_metaEntry = it->second;
            return true;
        }
    
        return false;
    }

    bool ResourceCatalog::GetCachedMetaDataForAsset(const std::string &p_assetPath,
        std::vector<MetaEntry> &p_metaData)
    {
        p_metaData.clear();
        auto it = m_assetToResourcesMap.find(p_assetPath);
        if (it != m_assetToResourcesMap.end())
        {
            for (GUID guid : it->second)
            {
                auto metaDataIt = m_guidToDataMap.find(guid);
                if (metaDataIt != m_guidToDataMap.end())
                {
                    p_metaData.push_back(metaDataIt->second);
                }
            }
            
            return true;
        }
        
        return false; // Asset is not registered
    }

    void ResourceCatalog::RegisterMetaEntry(const std::string &p_assetPath, const MetaEntry &p_metaEntry)
    {
        m_guidToDataMap[p_metaEntry.guid] = p_metaEntry;
        
        auto& guidList = m_assetToResourcesMap[p_assetPath];
        if (std::find(guidList.begin(), guidList.end(), p_metaEntry.guid) == guidList.end())
        {
            guidList.push_back(p_metaEntry.guid);
        } 
    }
}


