#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <cassert>

#include "asset/ResourceHandle.hpp"
#include "asset/ResourceCatalog.hpp"

namespace Droplet
{
    class IResource;

    /// @brief Represents all states that a resource can have.
    enum class ResourceState
    {
        Unloaded,
        Queued,
        LoadingAsync,
        ReadyAsync,
        Uploading,
        Ready,
        Failed
    };

    /// @brief Contains relevant information about a specific loaded resource.
    struct ResourceRecord
    {
        std::shared_ptr<IResource> resource = nullptr;
        ResourceState state = ResourceState::Unloaded;
        std::atomic<uint32_t> refCount{0};
    };

    /// @brief Universal resource manager class.
    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        /// @brief Initializes the resource manager for a specific root directory.
        /// @param p_rootDirectory The root directory that containing all assets that the manager should be able to load
        /// resources from.
        void Initialize(std::filesystem::path p_rootDirectory);

        /// @brief Parses an asset file and generates a .meta file based on its internal resources.
        /// @param p_assetPath The path to the asset to be registered. Must be within 
        void RegisterAsset(const std::filesystem::path &p_assetPath);

        /// @brief Loads a resource specified by a guid.
        /// @tparam T The resource type.
        /// @param p_guid The guid of the resource
        /// @return A handle to the resource. Make sure to check its validity before use.
        template<typename T>
        ResourceHandle<T> LoadResource(GUID p_guid)
        {
            assert(m_isInitialized && "ResourceManager is not initialized.");
            
            MetaEntry metaEntry;
            if (!m_catalog.GetResourceMetaData(p_guid, metaEntry))
            {
                // Handle missing resource
                // TODO: Log this as a warning/error
                return ResourceHandle<T>(C_INVALID_GUID, this);
            }
            
            {
                std::lock_guard<std::mutex> lock(m_registryMutex);
                
                auto [it, wasInserted] = m_registry.try_emplace(p_guid);
                if (!wasInserted)
                {
                    // The asset was not inserted (already exists)
                    return ResourceHandle<T>(p_guid, this);
                }
                
                // Asset was just created in-place -> Update state
                it->second.state = ResourceState::Queued;
                it->second.refCount.store(0, std::memory_order_relaxed); // Incremented when handle is constructed
            }
            
            // TODO: Push load task to worker thread pool
            
            return ResourceHandle<T>(p_guid, this);
        }
        
        void AddRef(GUID p_guid);
        void ReleaseRef(GUID p_guid);
        
        ResourceState GetState(GUID p_guid);
        
        template<typename T>
        T *GetResource(GUID p_guid)
        {
            assert(m_isInitialized && "AssetManager is not initialized.");
            
            std::lock_guard<std::mutex> lock(m_registryMutex);
            
            auto it = m_registry.find(p_guid);
            if (it != m_registry.end() && it->second.state == ResourceState::Ready)
            {
                // Asset is ready to be used
                return static_cast<T*>(it->second.resource.get());
            }
            
            return nullptr;
        }
    
    private:
        bool m_isInitialized = false;
        ResourceCatalog m_catalog;
        
        std::unordered_map<GUID, ResourceRecord> m_registry;
        std::mutex m_registryMutex; 
        
        // TODO: Add thread pool / job system
    };
    
}

#include "asset/ResourceHandle.inl" // Included here to avoid circular definitions


