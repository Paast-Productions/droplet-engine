#pragma once

#include "asset/GUID.hpp"

namespace Droplet
{
    class ResourceManager;

    /// @brief Proxy object for a resource managed by a resource manager.
    /// @tparam T The resource type that the handle is referencing.
    template<typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;
        ResourceHandle(GUID p_guid, ResourceManager *p_resourceManager);
        ~ResourceHandle();
        
        ResourceHandle(const ResourceHandle &other);
        ResourceHandle& operator=(const ResourceHandle &other);
        ResourceHandle(ResourceHandle &&other) noexcept;
        ResourceHandle& operator=(ResourceHandle &&other) noexcept;
        
        /// @return True if the handle is valid, otherwise false.
        bool IsValid() const;

        /// @return The globally unique identifier for the resource the handle is referencing.
        GUID GetGUID() const;
        
        /// @return True if resource is ready to be used, otherwise false.
        bool IsReady() const;
        
        /// @return True if resource failed to load, otherwise false.
        bool HasFailed() const;
        
        /// @return The loaded resource stored within the central resource manager, or nullptr if the load failed.
        T *Get() const;
        
    private:
        GUID m_guid = 0;
        ResourceManager *m_assetManager = nullptr;

        /// @brief Increments the reference of the resource the handle is referencing.
        void IncrementRef();

        /// @brief Decrements the reference count of the resource the handle is referencing.
        void DecrementRef();
    };
    
}

#include "asset/ResourceManager.hpp"
