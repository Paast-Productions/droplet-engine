#include "Asset/ResourceManager.hpp"

#include "meta/MetaUtils.hpp"

// #include "IResource"

namespace Droplet
{
    void ResourceManager::Initialize(std::filesystem::path p_rootDirectory)
    {
        if (m_isInitialized)
        {
            // Log RM already initialized
            return;
        }
        
        if (!m_catalog.ScanDirectory(p_rootDirectory))
        {
            // Log error (directory does not exist)
            m_isInitialized = false;
            return;
        }
        
        m_isInitialized = true;
    }

    void ResourceManager::RegisterAsset(const std::filesystem::path &p_assetPath)
    {
        std::vector<MetaEntry> metaEntry;
        
        std::filesystem::path metaPath = p_assetPath.string() + ".meta";
        if (std::filesystem::exists(metaPath))
        {
            std::string assetPathStr = p_assetPath.string();
            std::vector<MetaEntry> cachedMetaData;
            
            m_catalog.GetCachedMetaDataForAsset(assetPathStr, cachedMetaData);
            
            // Thread pool dispatch here with lambda[this, p_assetPath, assetPathStr, cachedMetaData]
            // --- Async Worker Thread ---
            std::vector<MetaEntry> discoveredResources;
            
            // Run assimp/whatever to detect resources in asset file
            
            // Dispatch back to main thread with lambda[this, assetPathStr, resourceList, cachedMetaData]
            std::vector<MetaEntry> mergedMetaData;
            
            // Use private function to compare cached metadata to resource list and append/remove bodies here
            
            // Overwrite .meta file with result (dispatch I/O thread to perform this task)
            // --- Async I/O Thread --- 
            std::filesystem::path writePath = assetPathStr + ".meta";
            MetaUtils::Write(writePath, mergedMetaData);
            
            // --- Main Thread ---
            // Update internal catalog
            for (const auto &entry : mergedMetaData)
            {
                m_catalog.RegisterMetaEntry(assetPathStr, entry);
            }
        }
    }

    void ResourceManager::IncrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1); // TODO: Look into which memory_order to use here
        }
    }

    void ResourceManager::DecrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            if (it->second.refCount.fetch_sub(1) == 1) // TODO: Look into which memory_order to use here
            {
                m_registry.erase(it);
            }
        }
    }

    ResourceState ResourceManager::GetState(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex); // This is not ideal for polling (performance)
            
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            return it->second.state;
        }
            
        return ResourceState::Unloaded;
    }

    std::vector<MetaEntry> ResourceManager::CompareAndCompileMetaData(const std::vector<MetaEntry> &p_metaData,
        const std::vector<std::pair<ResourceType, std::string>> &p_foundResources)
    {
        std::vector<MetaEntry> out;
        out.reserve(p_foundResources.size());
        
        // Iterate over the resources that were found in the asset
        for (const auto& [foundType, foundName] : p_foundResources)
        {
            // Try to find an existing match in the old metadata
            auto it = std::find_if(p_metaData.begin(), p_metaData.end(),
                [&](const MetaEntry &existingEntry)
                {
                    return existingEntry.type == foundType && existingEntry.name == foundName;
                }
            );
            
            if (it != p_metaData.end())
            {
                // Resource already exists in the old metadata -> copy it to the new metadata
                out.push_back(*it);
            }
            else
            {
                // Resource does not exist in the old metadata -> create a new entry for it
                out.push_back(MetaUtils::GenerateDefaultMetaEntry(foundType, foundName));
            }
        }
        
        return out;
    }
}
