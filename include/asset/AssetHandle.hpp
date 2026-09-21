#pragma once

#include <cstdint>

namespace Droplet
{
    class AssetManager;
    using GUID = uint64_t;

    /// @brief Proxy object for a resource managed by an asset manager.
    /// @tparam T The resource type that the handle is referencing.
    template<typename T>
    class AssetHandle
    {
    public:
        AssetHandle() = default;
        AssetHandle(GUID p_guid, AssetManager *p_assetManager);
        ~AssetHandle();
        
        AssetHandle(const AssetHandle &other);
        AssetHandle& operator=(const AssetHandle &other);
        AssetHandle(AssetHandle &&other) noexcept;
        AssetHandle& operator=(AssetHandle &&other) noexcept;
        
        /// @return True if the handle is valid, otherwise false.
        bool IsValid() const;

        /// @return The globally unique identifier for the asset the handle is referencing.
        GUID GetGUID() const;
        
        /// @return True if asset is ready to be used, otherwise false.
        bool IsReady() const;
        
        /// @return True if asset failed to load, otherwise false.
        bool HasFailed() const;
        
        /// @return The loaded asset stored within the central asset manager, or nullptr if the load failed.
        T *Get() const;
        
    private:
        GUID m_guid = 0;
        AssetManager *m_assetManager = nullptr;

        /// @brief Increments the reference of the asset the handle is referencing.
        void AddRef();

        /// @brief Decrements the reference count of the asset the handle is referencing.
        void ReleaseRef();
    };
    
}

#include "asset/AssetManager.hpp"
