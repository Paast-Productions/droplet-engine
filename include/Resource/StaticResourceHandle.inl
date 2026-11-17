#pragma once

#include "StaticResourceHandle.hpp"
#include <Resource/ResourceManager.hpp>

namespace Droplet
{
    template<typename T>
    StaticResourceHandle<T>::StaticResourceHandle(GUID p_guid, ResourceManager* p_resourceManager)
        : ResourceHandle<T>(p_guid), m_resourceManager(p_resourceManager)
    {
        StaticResourceHandle<T>::IncrementRef();
    }

    template <typename T>
    StaticResourceHandle<T>::~StaticResourceHandle()
    {
        StaticResourceHandle<T>::DecrementRef();
    }
    
    template <typename T>
    StaticResourceHandle<T>::StaticResourceHandle(const StaticResourceHandle &p_other)
        : ResourceHandle<T>(p_other.m_guid), m_resourceManager(p_other.m_resourceManager)
    {
        // Copy construct (+1 ref)
        StaticResourceHandle<T>::IncrementRef();
    }

    template <typename T>
    StaticResourceHandle<T> & StaticResourceHandle<T>::operator=(const StaticResourceHandle &p_other)
    {
        // Copy assign (new: +1 old: -1)
        if (this != &p_other)
        {
            DecrementRef();
            ResourceHandle<T>::m_guid = p_other.ResourceHandle<T>::m_guid;
            m_resourceManager = p_other.m_resourceManager;
            IncrementRef();
        }
            
        return *this;
    }

    template <typename T>
    StaticResourceHandle<T>::StaticResourceHandle(StaticResourceHandle &&p_other) noexcept
        : ResourceHandle<T>(p_other.m_guid), m_resourceManager(p_other.m_resourceManager) 
    {
        // Move construct (steal data, no ref change)
        p_other.m_guid = C_INVALID_GUID;
        p_other.m_resourceManager = nullptr;
    }

    template <typename T>
    StaticResourceHandle<T> & StaticResourceHandle<T>::operator=(StaticResourceHandle &&p_other) noexcept
    {
        // Move assign (steal to new, no ref change)
        if (this != &p_other)
        {
            DecrementRef();
            ResourceHandle<T>::m_guid = p_other.m_guid;
            m_resourceManager = p_other.m_resourceManager;
                
            p_other.m_guid = C_INVALID_GUID;
            p_other.m_resourceManager = nullptr;
        }
            
        return *this;   
    }

    template <typename T>
    bool StaticResourceHandle<T>::IsValid() const
    {
        return ResourceHandle<T>::m_guid != C_INVALID_GUID && m_resourceManager;
    }

    template <typename T>
    bool StaticResourceHandle<T>::IsQueued() const
    {
        if (!IsValid())
        {
            return false;
        }
        
        return m_resourceManager->GetState(ResourceHandle<T>::m_guid) == ResourceState::Queued;
    }

    template <typename T>
    bool StaticResourceHandle<T>::IsLoading() const
    {
        if (!IsValid)
        {
            return false;
        }
        
        return m_resourceManager->GetState(ResourceHandle<T>::m_guid) == ResourceState::LoadingAsync;
    }

    template <typename T>
    bool StaticResourceHandle<T>::IsReady() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_resourceManager->GetState(ResourceHandle<T>::m_guid) == ResourceState::Ready; 
    }

    template <typename T>
    bool StaticResourceHandle<T>::HasFailed() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_resourceManager->GetState(ResourceHandle<T>::m_guid) == ResourceState::Failed;
    }

    template <typename T>
    T *StaticResourceHandle<T>::Get() const
    {
        if (!m_resourceManager)
        {
            return nullptr;
        }
        return m_resourceManager->GetResource<T>(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    void StaticResourceHandle<T>::IncrementRef()
    {
        if (IsValid())
        {
            m_resourceManager->IncrementRef(ResourceHandle<T>::m_guid);
        }
    }

    template <typename T>
    void StaticResourceHandle<T>::DecrementRef()
    {
        if (IsValid())
        {
            m_resourceManager->DecrementRef(ResourceHandle<T>::m_guid);
        }
    }

    template <typename T>
    std::uint32_t StaticResourceHandle<T>::GetRef() const
    {
        if (IsValid())
        {
            return m_resourceManager->GetRef(ResourceHandle<T>::m_guid);
        }
        else
        {
            return 0;
        }
    }
}
