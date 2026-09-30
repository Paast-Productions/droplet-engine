#pragma once

#include "resource/ResourceHandle.hpp"
#include "resource/ResourceManager.hpp"

namespace Droplet
{
    template<typename T>
    ResourceHandle<T>::ResourceHandle(GUID p_guid, ResourceManager* p_resourceManager)
        : m_guid(p_guid), m_resourceManager(p_resourceManager)
    {
        IncrementRef();
    }

    template <typename T>
    ResourceHandle<T>::~ResourceHandle()
    {
        DecrementRef();
    }
    
    template <typename T>
    ResourceHandle<T>::ResourceHandle(const ResourceHandle &p_other)
        : m_guid(p_other.m_guid), m_resourceManager(p_other.m_resourceManager)
    {
        // Copy construct (+1 ref)
        IncrementRef();
    }

    template <typename T>
    ResourceHandle<T> & ResourceHandle<T>::operator=(const ResourceHandle &p_other)
    {
        // Copy assign (new: +1 old: -1)
        if (this != &p_other)
        {
            DecrementRef();
            m_guid = p_other.m_guid;
            m_resourceManager = p_other.m_resourceManager;
            IncrementRef();
        }
            
        return *this;
    }

    template <typename T>
    ResourceHandle<T>::ResourceHandle(ResourceHandle &&p_other) noexcept
        : m_guid(p_other.m_guid), m_resourceManager(p_other.m_resourceManager) 
    {
        // Move construct (steal data, no ref change)
        p_other.m_guid = C_INVALID_GUID;
        p_other.m_resourceManager = nullptr;
    }

    template <typename T>
    ResourceHandle<T> & ResourceHandle<T>::operator=(ResourceHandle &&p_other) noexcept
    {
        // Move assign (steal to new, no ref change)
        if (this != &p_other)
        {
            DecrementRef();
            m_guid = p_other.m_guid;
            m_resourceManager = p_other.m_resourceManager;
                
            p_other.m_guid = C_INVALID_GUID;
            p_other.m_resourceManager = nullptr;
        }
            
        return *this;   
    }

    template <typename T>
    bool ResourceHandle<T>::IsValid() const
    {
        return m_guid != C_INVALID_GUID && m_resourceManager;
    }

    template <typename T>
    GUID ResourceHandle<T>::GetGUID() const
    {
        return m_guid;
    }

    template <typename T>
    bool ResourceHandle<T>::IsReady() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_resourceManager->GetState(m_guid) == ResourceState::Ready; 
    }

    template <typename T>
    bool ResourceHandle<T>::HasFailed() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_resourceManager->GetState(m_guid) == ResourceState::Failed;
    }

    template <typename T>
    T *ResourceHandle<T>::Get() const
    {
        if (IsValid())
        {
            return m_resourceManager->GetResource<T>(m_guid);
        }
            
        return nullptr;
    }

    template <typename T>
    void ResourceHandle<T>::IncrementRef()
    {
        if (IsValid())
        {
            m_resourceManager->IncrementRef(m_guid);
        }
    }

    template <typename T>
    void ResourceHandle<T>::DecrementRef()
    {
        if (IsValid())
        {
            m_resourceManager->DecrementRef(m_guid);
        }
    }

    template <typename T>
    uint32_t ResourceHandle<T>::GetRef()
    {
        if (IsValid())
        {
            return m_resourceManager->GetRef(m_guid);
        }
        else
        {
            return 0;
        }
    }
}
