#include "resource/ResourceRegistry.hpp"

#include <Windows.h>

#include "resource/meta/MetaUtils.hpp"

namespace Droplet
{
    bool ResourceRegistry::ScanDirectory(const std::filesystem::path &p_directory)
    {
        if (!std::filesystem::is_directory(p_directory))
        {
            return false;        
        }
        
        for (const auto &item : std::filesystem::recursive_directory_iterator(p_directory))
        {
            if (item.is_regular_file() && item.path().extension() == ".meta")
            {
                std::filesystem::path metaPath = item.path();
                metaPath.replace_extension(""); // Remove ".meta" from the path
                
                std::string assetPath = metaPath.generic_string();
                
                std::vector<MetaEntry> entries;
                if (MetaUtils::Read(item.path(), entries))
                {   
                    for (const auto& entry : entries)
                    {
                        RegisterMetaEntry(assetPath, entry);
                    }
                }
                else
                {
                    // TODO: Log metafile parse error (or empty)
                }
            }
        }
        
        return true;
    }

    bool ResourceRegistry::GetResourceMetaData(GUID p_guid, MetaEntry &p_metaEntry)
    {
        auto it = m_guidToEntryMap.find(p_guid);
        if (it != m_guidToEntryMap.end())
        {
            p_metaEntry = it->second;
            return true;
        }
    
        return false;
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

    void ResourceRegistry::RegisterMetaEntry(const std::string &p_assetPath, const MetaEntry &p_metaEntry)
    {
        m_guidToEntryMap[p_metaEntry.guid] = p_metaEntry;
        
        auto& guidList = m_assetToResourcesMap[p_assetPath];
        if (std::find(guidList.begin(), guidList.end(), p_metaEntry.guid) == guidList.end())
        {
            guidList.push_back(p_metaEntry.guid);
        } 
    }
}


