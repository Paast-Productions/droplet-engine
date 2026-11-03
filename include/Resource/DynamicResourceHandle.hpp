#pragma once

namespace Droplet
{
    class IDynamicResource;
    
    template <typename>
    class ResourceHandle;
}



namespace Droplet
{
    template <typename T>
    class DynamicResourceHandle<T> : public ResourceHandle<T>
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
        
        T* operator->();
        
        bool IsValid() const override;
        T* Get() const override;
        std::uint32_t GetRef() const override;
        
    protected:
        void IncrementRef() override;
        
        void DecrementRef() override;
        
    private:
    };
}

#include <Resource/DynamicResourceHandle.inl>
