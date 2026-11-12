#pragma once

#include <Graphics/VK/IntermediaryStorage.hpp>

namespace Droplet
{
    template <typename T>
    DynamicResourceHandle<T>::DynamicResourceHandle(GUID p_guid) : 
        ResourceHandle<T>(p_guid)
    {
        Graphics::VK::IntermediaryStorage::GetInstance().IncrementRef(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    DynamicResourceHandle<T>::DynamicResourceHandle(const DynamicResourceHandle &p_other) :
        ResourceHandle<T>(p_other.ResourceHandle<T>::m_guid)
    {
        if (ResourceHandle<T>::m_guid == C_INVALID_GUID)
        {
            return;    
        }

        Graphics::VK::IntermediaryStorage::GetInstance().IncrementRef(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    DynamicResourceHandle<T> &DynamicResourceHandle<T>::operator=(const DynamicResourceHandle &p_other)
    {
        if (*this == p_other)
        {
            return *this;
        }
        
        DecrementRef();
        ResourceHandle<T>::m_guid = p_other.ResourceHandle<T>::m_guid;
        IncrementRef();
        
        return *this;
    }

    template <typename T>
    DynamicResourceHandle<T>::DynamicResourceHandle(DynamicResourceHandle &&p_other) noexcept
    { 
        std::swap(ResourceHandle<T>::m_guid, p_other.m_guid);
        //p_other.m_guid = C_INVALID_GUID;
    }

    template <typename T>
    DynamicResourceHandle<T> &DynamicResourceHandle<T>::operator=(DynamicResourceHandle &&p_other) noexcept
    {
        if (*this == p_other)
        {
            return *this;    
        }   
     
        std::swap(ResourceHandle<T>::m_guid, p_other.m_guid);
        //p_other.m_guid = C_INVALID_GUID;
        
        return *this;
    }

    template <typename T>
    DynamicResourceHandle<T>::~DynamicResourceHandle()
    {
        if (ResourceHandle<T>::m_guid == C_INVALID_GUID)
        {
            return;
        }
        
        Graphics::VK::IntermediaryStorage::GetInstance().DecrementRef(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    bool DynamicResourceHandle<T>::operator==(const DynamicResourceHandle &p_other) const
    {
        return ResourceHandle<T>::m_guid == p_other.m_guid;
    }

    template <typename T>
    T *DynamicResourceHandle<T>::operator->() const
    {
        return Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObject<T>(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    bool DynamicResourceHandle<T>::IsValid() const
    {
        return ResourceHandle<T>::m_guid != C_INVALID_GUID;
    }
    
    //DynamicResourceHandle

    template <typename T>
    T *DynamicResourceHandle<T>::Get() const
    {
        return Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObject<T>(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    std::uint32_t DynamicResourceHandle<T>::GetRef() const
    {
        if (!IsValid())
        {
            return 0;
        }
        
        return Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    void DynamicResourceHandle<T>::IncrementRef()
    {
        if (!IsValid())
        {
            return;
        }
        
        Graphics::VK::IntermediaryStorage::GetInstance().IncrementRef(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    void DynamicResourceHandle<T>::DecrementRef()
    {
        if (!IsValid())
        {
            return;
        }
        
        Graphics::VK::IntermediaryStorage::GetInstance().DecrementRef(ResourceHandle<T>::m_guid);
    }
}