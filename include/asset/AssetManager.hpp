#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <cassert>

#include "asset/AssetHandle.hpp"
#include "asset/AssetCatalog.hpp"

namespace Droplet
{
    class IResource;

    /// @brief Represents all states that an asset can have.
    enum class AssetState
    {
        Unloaded,
        Queued,
        LoadingAsync,
        ReadyAsync,
        Uploading,
        Ready,
        Failed
    };

    /// @brief Contains relevant information about a specific loaded asset.
    struct AssetRecord
    {
        std::shared_ptr<IResource> resource = nullptr;
        AssetState state = AssetState::Unloaded;
        std::atomic<uint32_t> refCount{0};
    };

    /// @brief Central, universal asset manager class used to load all types of assets.
    class AssetManager
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        /// @brief Initializes the asset manager for a specific root directory.
        /// @param p_rootDirectory The root directory that containing all assets that the manager should be able to load.
        void Initialize(std::filesystem::path p_rootDirectory);

        /// @brief Loads an asset specified by a path relative to the root directory of the asset manager.
        /// @tparam T The asset type.
        /// @param p_path The path of the asset relative to the root directory of the asset manager
        /// @return An AssetHandle. Make sure to check its validity before use.
        template<typename T>
        AssetHandle<T> Load(const std::string& p_path)
        {
            assert(m_isInitialized && "AssetManager is not initialized.");
            
            GUID guid = m_catalog.GetGUID(p_path);
            if (guid == 0)
            {
                // Handle missing asset
                // Log this as a warning/error
                return AssetHandle<T>(guid, this);
            }
            
            {
                std::lock_guard<std::mutex> lock(m_registryMutex);
                
                auto [it, wasInserted] = m_registry.try_emplace(guid);
                if (!wasInserted)
                {
                    // The asset was not inserted (already exists)
                    return AssetHandle<T>(guid, this);
                }
                
                // Asset was just created in-place -> Update state
                it->second.state = AssetState::Queued;
                it->second.refCount.store(0, std::memory_order_relaxed); // Incremented when handle is constructed
            }
            
            // TODO: Push load task to worker thread pool
            
            return AssetHandle<T>(guid, this);
        }
        
        void AddRef(GUID p_guid);
        void ReleaseRef(GUID p_guid);
        
        AssetState GetState(GUID p_guid);
        
        template<typename T>
        T *GetResource(GUID p_guid)
        {
            assert(m_isInitialized && "AssetManager is not initialized.");
            
            std::lock_guard<std::mutex> lock(m_registryMutex);
            
            auto it = m_registry.find(p_guid);
            if (it != m_registry.end() && it->second.state == AssetState::Ready)
            {
                // Asset is ready to be used
                return static_cast<T*>(it->second.resource.get());
            }
            
            return nullptr;
        }
    
    private:
        bool m_isInitialized = false;
        AssetCatalog m_catalog;
        
        std::unordered_map<GUID, AssetRecord> m_registry;
        std::mutex m_registryMutex; 
        
        // TODO: Add thread pool / job system
    };
    
}

#include "asset/AssetHandle.inl" // Included here to avoid circular definitions


