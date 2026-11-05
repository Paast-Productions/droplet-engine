#pragma once

#include <Resource/ResourceHandle.hpp>

namespace Droplet
{
    // Forward declarations
    class IDynamicResource;
}

namespace Droplet
{
    template <typename T>
    class DynamicResourceHandle : public ResourceHandle<T>
    {
    public:
        static_assert(std::derived_from<T, IDynamicResource>);
        DynamicResourceHandle() = default;
        
        DynamicResourceHandle(GUID p_guid);
    
        DynamicResourceHandle(const DynamicResourceHandle &p_other);
        DynamicResourceHandle &operator=(const DynamicResourceHandle &p_other);
        DynamicResourceHandle(DynamicResourceHandle &&p_other) noexcept;
        DynamicResourceHandle &operator=(DynamicResourceHandle &&p_other) noexcept;

        ~DynamicResourceHandle() override;
        
        T* operator->() const;
        
        // TODO: Refactor to be private with intermediary storage friended.
        // This will make it so that the vulkan resource is exclusively restricted to the rendering pipeline
        T* Get() const override;
        bool IsValid() const override;
        std::uint32_t GetRef() const override;
        
    protected:
        void IncrementRef() override;
        
        void DecrementRef() override;
        
    private:
    };
}

#include "Resource/DynamicResourceHandle.inl"
