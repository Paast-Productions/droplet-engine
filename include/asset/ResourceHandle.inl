#pragma once

#include "asset/ResourceHandle.hpp"
#include "asset/ResourceManager.hpp"

namespace Droplet
{
    template<typename T>
    ResourceHandle<T>::ResourceHandle(GUID p_guid, ResourceManager* p_resourceManager)
        : m_guid(p_guid), m_assetManager(p_resourceManager)
    {
        IncrementRef();
    }

    template <typename T>
    ResourceHandle<T>::~ResourceHandle()
    {
        DecrementRef();
    }
    
    template <typename T>
    ResourceHandle<T>::ResourceHandle(const ResourceHandle &other)
        : m_guid(other.m_guid), m_assetManager(other.m_assetManager)
    {
        // Copy construct (+1 ref)
        IncrementRef();
    }

    template <typename T>
    ResourceHandle<T> & ResourceHandle<T>::operator=(const ResourceHandle &other)
    {
        // Copy assign (new: +1 old: -1)
        if (this != &other)
        {
            DecrementRef();
            m_guid = other.m_guid;
            m_assetManager = other.m_assetManager;
            IncrementRef();
        }
            
        return *this;
    }

    template <typename T>
    ResourceHandle<T>::ResourceHandle(ResourceHandle &&other) noexcept
        : m_guid(other.m_guid), m_assetManager(other.m_assetManager) 
    {
        // Move construct (steal data, no ref change)
        other.m_guid = C_INVALID_GUID;
        other.m_assetManager = nullptr;
    }

    template <typename T>
    ResourceHandle<T> & ResourceHandle<T>::operator=(ResourceHandle &&other) noexcept
    {
        // Move assign (steal to new, no ref change)
        if (this != &other)
        {
            DecrementRef();
            m_guid = other.m_guid;
            m_assetManager = other.m_assetManager;
                
            other.m_guid = C_INVALID_GUID;
            other.m_assetManager = nullptr;
        }
            
        return *this;   
    }

    template <typename T>
    bool ResourceHandle<T>::IsValid() const
    {
        return m_guid != C_INVALID_GUID && m_assetManager;
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
            
        return m_assetManager->GetState(m_guid) == ResourceState::Ready; 
    }

    template <typename T>
    bool ResourceHandle<T>::HasFailed() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_assetManager->GetState(m_guid) == ResourceState::Failed;
    }

    template <typename T>
    T *ResourceHandle<T>::Get() const
    {
        if (IsValid())
        {
            return m_assetManager->GetResource<T>(m_guid);
        }
            
        return nullptr;
    }

    template <typename T>
    void ResourceHandle<T>::IncrementRef()
    {
        if (IsValid())
        {
            m_assetManager->IncrementRef(m_guid);
        }
    }

    template <typename T>
    void ResourceHandle<T>::DecrementRef()
    {
        if (IsValid())
        {
            m_assetManager->DecrementRef(m_guid);
        }
    }
}
