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
    }
}


