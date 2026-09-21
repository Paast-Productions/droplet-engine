#include "Asset/ResourceManager.hpp"

#include "meta/MetaSerializer.hpp"

// #include "IResource"

namespace Droplet
{
    void ResourceManager::Initialize(std::filesystem::path p_rootDirectory)
    {
        if (m_isInitialized)
        {
            // Log AM already initialized
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
            std::filesystem::path metaPath = assetPathStr + ".meta";
            MetaSerializer::Write(metaPath, mergedMetaData);
            
            // --- Main Thread ---
            // Update internal catalog
            for (const auto &entry : mergedMetaData)
            {
                m_catalog.RegisterMetaEntry(assetPathStr, entry);
            }
        }
    }

    void ResourceManager::AddRef(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1); // TODO: Look into which memory_order to use here
        }
    }

    void ResourceManager::ReleaseRef(GUID p_guid)
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
}
