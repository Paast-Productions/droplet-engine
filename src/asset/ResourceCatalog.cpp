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

    bool ResourceCatalog::GetResourceMetaData(GUID p_guid, ResourceMetaData &p_resourceData)
    {
        auto it = m_metaDataMap.find(p_guid);
        if (it != m_metaDataMap.end())
        {
            p_resourceData = it->second;
            return true;
        }
    
        return false;
    }
    }
}


