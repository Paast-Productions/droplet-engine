#pragma once

#include "asset/AssetHandle.hpp"
#include "asset/AssetManager.hpp"

namespace droplet
{
    template<typename T>
    AssetHandle<T>::AssetHandle(GUID p_guid, AssetManager* p_assetManager)
        : m_guid(p_guid), m_assetManager(p_assetManager)
    {
        AddRef();
    }

    template <typename T>
    AssetHandle<T>::~AssetHandle()
    {
        ReleaseRef();
    }
    
    template <typename T>
    AssetHandle<T>::AssetHandle(const AssetHandle &other)
        : m_guid(other.m_guid), m_assetManager(other.m_assetManager)
    {
        // Copy construct (+1 ref)
        AddRef();
    }

    template <typename T>
    AssetHandle<T> & AssetHandle<T>::operator=(const AssetHandle &other)
    {
        // Copy assign (new: +1 old: -1)
        if (this != &other)
        {
            ReleaseRef();
            m_guid = other.m_guid;
            m_assetManager = other.m_assetManager;
            AddRef();
        }
            
        return *this;
    }

    template <typename T>
    AssetHandle<T>::AssetHandle(AssetHandle &&other) noexcept
        : m_guid(other.m_guid), m_assetManager(other.m_assetManager) 
    {
        // Move construct (steal data, no ref change)
        other.m_guid = 0;
        other.m_assetManager = nullptr;
    }

    template <typename T>
    AssetHandle<T> & AssetHandle<T>::operator=(AssetHandle &&other) noexcept
    {
        // Move assign (steal to new, no ref change)
        if (this != &other)
        {
            ReleaseRef();
            m_guid = other.m_guid;
            m_assetManager = other.m_assetManager;
                
            other.m_guid = 0;
            other.m_assetManager = nullptr;
        }
            
        return *this;   
    }

    template <typename T>
    bool AssetHandle<T>::IsValid() const
    {
        return m_guid != 0 && m_assetManager;
    }

    template <typename T>
    GUID AssetHandle<T>::GetGUID() const
    {
        return m_guid;
    }

    template <typename T>
    bool AssetHandle<T>::IsReady() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_assetManager->GetState(m_guid) == AssetState::Ready; 
    }

    template <typename T>
    bool AssetHandle<T>::HasFailed() const
    {
        if (!IsValid())
        {
            return false;
        }
            
        return m_assetManager->GetState(m_guid) == AssetState::Failed;
    }

    template <typename T>
    T *AssetHandle<T>::Get() const
    {
        if (IsValid())
        {
            return m_assetManager->GetResource<T>(m_guid);
        }
            
        return nullptr;
    }

    template <typename T>
    void AssetHandle<T>::AddRef()
    {
        if (IsValid())
        {
            m_assetManager->AddRef(m_guid);
        }
    }

    template <typename T>
    void AssetHandle<T>::ReleaseRef()
    {
        if (IsValid())
        {
            m_assetManager->ReleaseRef(m_guid);
        }
    }
}
