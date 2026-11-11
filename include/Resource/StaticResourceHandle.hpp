#pragma once

#include <Resource/ResourceHandle.hpp>
#include <Resource/GuidUtils.hpp>

namespace Droplet
{
    class ResourceManager;

    /// @brief Proxy object for a resource managed by a resource manager.
    /// @tparam T The resource type that the handle is referencing.
    template<typename T>
    class StaticResourceHandle : public ResourceHandle<T>
    {
    public:
        static_assert(std::derived_from<T, IStaticResource>);
        
        StaticResourceHandle() = default;
        StaticResourceHandle(GUID p_guid, ResourceManager *p_resourceManager);
        
        StaticResourceHandle(const StaticResourceHandle &other);
        StaticResourceHandle& operator=(const StaticResourceHandle &p_other);
        StaticResourceHandle(StaticResourceHandle &&p_other) noexcept;
        StaticResourceHandle& operator=(StaticResourceHandle &&p_other) noexcept;
        
        ~StaticResourceHandle() override;

        /// @return True if resource is queued, otherwise false.
        bool IsQueued() const;
        
        /// @return True if resource is currently loading, otherwise false.
        bool IsLoading() const; 
        
        /// @return True if resource is ready to be used, otherwise false.
        bool IsReady() const;
        
        /// @return True if resource failed to load, otherwise false.
        bool HasFailed() const;
        
        /// @return True if the handle is valid, otherwise false.
        bool IsValid() const override;
        
        /// @return The loaded resource stored within the central resource manager, or nullptr if the load failed.
        T *Get() const override;
        
        std::uint32_t GetRef() const override;
        
    protected:
        /// @brief Increments the reference of the resource the handle is referencing.
        void IncrementRef() override;

        /// @brief Decrements the reference count of the resource the handle is referencing.
        void DecrementRef() override;
        
    private:
        ResourceManager *m_resourceManager = nullptr;

    };
    
}

#include "resource/ResourceManager.hpp"
