#include "Asset/AssetCatalog.hpp"

namespace droplet
{
    bool AssetCatalog::ScanDirectory(const std::filesystem::path &p_directory)
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

    GUID AssetCatalog::GetGUID(const std::string &p_path)
    {
        auto it = m_pathToGuidMap.find(p_path);
        if (it != m_pathToGuidMap.end())
        {
            return it->second;
        }
    
        return 0; // Invalid guid
    }
}


