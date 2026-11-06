#pragma once

#include <Graphics/VK/IntermediaryStorage.hpp>

namespace Droplet
{
    template <typename T>
    DynamicResourceHandle<T>::DynamicResourceHandle(GUID p_guid) : 
        ResourceHandle<T>(p_guid)
    {
        std::print(std::cout, "Increment (GUID-Constructor): {0}, Current refcount: {1}\n", ResourceHandle<T>::m_guid, Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid) + 1);
        std::flush(std::cout);
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
        
        std::print(std::cout, "Increment (Copy Constructor): {0}, Current refcount: {1}\n", ResourceHandle<T>::m_guid, Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid) + 1);
        std::flush(std::cout);
        Graphics::VK::IntermediaryStorage::GetInstance().IncrementRef(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    DynamicResourceHandle<T> &DynamicResourceHandle<T>::operator=(const DynamicResourceHandle &p_other)
    {
        if (this == &p_other)
        {
            return *this;
        }
        
        std::print(std::cout, "Decrement (Copy Assignment): {0}, Current refcount: {1}\n", ResourceHandle<T>::m_guid, Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid) - 1);
        std::flush(std::cout);
        DecrementRef();
        ResourceHandle<T>::m_guid = p_other.ResourceHandle<T>::m_guid;
        std::print(std::cout, "Increment (Copy Assignment): {0}, Current refcount: {1}\n", ResourceHandle<T>::m_guid, Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid) + 1);
        std::flush(std::cout);
        IncrementRef();
        
        return *this;
    }

    template <typename T>
    DynamicResourceHandle<T>::DynamicResourceHandle(DynamicResourceHandle &&p_other) noexcept :
        ResourceHandle<T>(p_other.m_guid)
    {
        std::swap(ResourceHandle<T>::m_guid, p_other.m_guid);
    }

    template <typename T>
    DynamicResourceHandle<T> &DynamicResourceHandle<T>::operator=(DynamicResourceHandle &&p_other) noexcept
    {
        if (this == &p_other)
        {
            return *this;    
        }   
        
        std::swap(ResourceHandle<T>::m_guid, p_other.m_guid);
        
        return *this;
    }

    template <typename T>
    T *DynamicResourceHandle<T>::operator->() const
    {
        return Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObject<T>(ResourceHandle<T>::m_guid);
    }

    template <typename T>
    DynamicResourceHandle<T>::~DynamicResourceHandle()
    {
        if (ResourceHandle<T>::m_guid == C_INVALID_GUID)
        {
            return;
        }
        
        std::print(std::cout, "Decrement (Destructor): {0}, Current refcount: {1}\n", ResourceHandle<T>::m_guid, Graphics::VK::IntermediaryStorage::GetInstance().GetLiveObjectRefCount(ResourceHandle<T>::m_guid) - 1);
        std::flush(std::cout);
        Graphics::VK::IntermediaryStorage::GetInstance().DecrementRef(ResourceHandle<T>::m_guid);
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