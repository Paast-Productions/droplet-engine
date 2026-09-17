#include "Asset/AssetManager.hpp"

// #include "IResource"

namespace engine
{
    void AssetManager::Initialize(std::filesystem::path p_rootDirectory)
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

    void AssetManager::AddRef(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1); // TODO: Look into which memory_order to use here
        }
    }

    void AssetManager::ReleaseRef(GUID p_guid)
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

    AssetState AssetManager::GetState(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex); // This is not ideal for polling (performance)
            
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            return it->second.state;
        }
            
        return AssetState::Unloaded;
    }
}
