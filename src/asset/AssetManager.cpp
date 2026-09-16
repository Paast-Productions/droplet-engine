#include "Asset/AssetManager.h"

#include "Asset/AssetHandle.h"
// #include "IResource"

namespace engine
{
    void AssetManager::AddRef(GUID p_guid)
    {
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1);
        }
    }

    void AssetManager::ReleaseRef(GUID p_guid)
    {
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            if (it->second.refCount.fetch_sub(1) == 1)
            {
                m_registry.erase(it);
            }
        }
    }
}
