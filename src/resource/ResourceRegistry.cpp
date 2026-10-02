#include "resource/ResourceRegistry.hpp"

#include "resource/meta/MetaUtils.hpp"

namespace Droplet
{
    void ResourceRegistry::ScanDirectory(const std::filesystem::path &p_directory)
    {
        if (!std::filesystem::is_directory(p_directory))
        {
            throw std::runtime_error("Directory does not exist.");        
        }
        
        for (const auto &item : std::filesystem::recursive_directory_iterator(p_directory))
        {
            if (item.is_regular_file() && item.path().extension() == ".meta")
            {
                std::vector<MetaEntry> entries;
                if (MetaUtils::Read(item.path(), entries))
                {   
                    // Calculate the relative asset path
                    std::filesystem::path absAssetPath = item.path();
                    absAssetPath.replace_extension(""); // Drop .meta
                    std::string relAssetPath = std::filesystem::relative(absAssetPath, p_directory).generic_string();
                    
                    for (auto &entry : entries)
                    {
                        entry.relAssetPath = relAssetPath;
                        RegisterMetaEntry(entry.relAssetPath, entry);
                    }
                }
                else
                {
                    // TODO: Log metafile parse error (or empty)
                }
            }
        }
    }

    MetaEntry ResourceRegistry::GetResourceMetaData(GUID p_guid)
    {
        auto it = m_guidToEntryMap.find(p_guid);
        if (it != m_guidToEntryMap.end())
        {
            return it->second;
        }
    
        throw std::runtime_error("Resource does not exist in registry.");
    }

    bool ResourceRegistry::GetCachedMetaDataForAsset(const std::string &p_assetPath,
                                                     std::vector<MetaEntry> &p_metaData)
    {
        p_metaData.clear();
        auto it = m_assetToResourcesMap.find(p_assetPath);
        if (it != m_assetToResourcesMap.end())
        {
            for (GUID guid : it->second)
            {
                auto metaDataIt = m_guidToEntryMap.find(guid);
                if (metaDataIt != m_guidToEntryMap.end())
                {
                    p_metaData.push_back(metaDataIt->second);
                }
            }
            
            return true;
        }
        
        return false; // Asset is not registered
    }

    std::vector<const MetaEntry *> ResourceRegistry::GetEntries(ResourceType p_type) const
    {
        std::vector<const MetaEntry *> result;
        result.reserve(m_guidToEntryMap.size());
        
        for (const auto &[guid, entry] : m_guidToEntryMap)
        {
            if (p_type == ResourceType::None || entry.type == p_type)
            {
                result.push_back(&entry);
            }
        }
        
        return result;
    }

    void ResourceRegistry::RegisterMetaEntry(const std::string &p_relAssetPath, const MetaEntry &p_metaEntry)
    {
        m_guidToEntryMap[p_metaEntry.guid] = p_metaEntry;
        
        auto &guidList = m_assetToResourcesMap[p_relAssetPath];
        if (std::find(guidList.begin(), guidList.end(), p_metaEntry.guid) == guidList.end())
        {
            guidList.push_back(p_metaEntry.guid);
        } 
    }
}


