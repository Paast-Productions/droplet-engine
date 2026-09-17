#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>

#include "Asset/AssetCatalog.hpp"

namespace engine
{
    class IResource;
    template <typename T> class AssetHandle;

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
            GUID guid = m_catalog.GetGUID(p_path);
            if (guid == 0)
            {
                // Handle missing asset
                // Log this as a warning/error
                return AssetHandle<T>(guid, this);
            }
            
            {
                std::lock_guard<std::mutex> lock(m_registryMutex);
                
                auto it = m_registry.find(guid);
                if (it != m_registry.end())
                {
                    // Asset already exists or is being loaded
                    return AssetHandle<T>(guid, this); // Ref count is incremented by AssetHandle constructor
                }
                
                // Asset is not loaded -> create record and add to registry
                AssetRecord record;
                record.state = AssetState::Queued;
                record.refCount.store(0);
                
                m_registry[guid] = std::move(record);
            }
            
            // TODO: Push load task to worker thread pool
            
            return AssetHandle<T>(guid, this);
        }
        
        void AddRef(GUID p_guid);
        void ReleaseRef(GUID p_guid);
        
        template<typename T>
        T* GetResource(GUID guid)
        {
            std::lock_guard<std::mutex> lock(m_registryMutex);
            
            auto it = m_registry.find(guid);
            if (it != m_registry.end() && it->second.state == AssetState::Ready)
            {
                // Asset is ready to be used
                return static_cast<T*>(it->second.resource.get());
            }
            
            return nullptr;
        }
    
    private:
        AssetCatalog m_catalog;
        
        std::unordered_map<GUID, AssetRecord> m_registry;
        std::mutex m_registryMutex;
        
        // TODO: Add thread pool / job system
    };
}


