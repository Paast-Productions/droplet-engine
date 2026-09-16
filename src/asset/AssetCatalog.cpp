#include "AssetCatalog.h"

namespace engine
{
    void AssetCatalog::ScanDirectory(const std::filesystem::path& p_directory)
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(p_directory))
        {
            if (entry.path().extension() == ".meta")
            {
                // Read json formatted meta file
                // Extract guid
                // Derive asset path from file name
                // Add to m_pathToGuid
            }
        }
    }

    GUID AssetCatalog::GetGUID(const std::string& p_path)
    {
        auto it = m_pathToGuid.find(p_path);
        if (it != m_pathToGuid.end())
        {
            return it->second;
        }
    
        return 0; // Invalid guid
    }
}


