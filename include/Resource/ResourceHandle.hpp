#pragma once

#include <Resource/IStaticResource.hpp>
#include <Resource/IDynamicResource.hpp>

namespace Droplet
{
    template<typename T>
    class ResourceHandle
    {
    public:
        using GUID = std::uint64_t;
        static_assert(std::derived_from<T, IStaticResource> || std::derived_from<T, IDynamicResource>);
        
        ResourceHandle() = default;
        
        ResourceHandle(GUID p_guid) :
            m_guid(p_guid) {}
        
        ResourceHandle(const ResourceHandle &p_other) = default;
        ResourceHandle &operator=(const ResourceHandle &p_other) = default;
        ResourceHandle(ResourceHandle &&p_other) noexcept = default;
        ResourceHandle &operator=(ResourceHandle &&p_other) noexcept = default;

        virtual ~ResourceHandle() = default;
        
        GUID GetGUID() const { return m_guid; }
        
        virtual bool IsValid() const = 0; 
        
        virtual T* Get() const = 0;
        virtual std::uint32_t GetRef() const = 0;
        
    protected:
        virtual void IncrementRef() = 0;
        virtual void DecrementRef() = 0;
        
        GUID m_guid {};
    };
}
