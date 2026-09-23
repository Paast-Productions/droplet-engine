#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <cassert>
#include <functional>
#include <iostream>

#include "asset/ResourceHandle.hpp"
#include "asset/ResourceCatalog.hpp"
#include "asset/Resource.hpp"
#include "asset/ThreadPool.hpp"
#include "asset/TextureLoader.hpp"

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
        std::unique_ptr<IResource> resource = nullptr;
        ResourceState state = ResourceState::Unloaded;
        std::atomic<uint32_t> refCount{0};
        
        std::vector<std::function<void()>> loadCallbacks; // Called when resource hits state: Ready
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
        void Initialize(const std::filesystem::path &p_rootDirectory);

        // TODO: This function should process uploads and trigger callbacks (and should be called in main update loop)
        // void Update();

        /// @brief Parses an asset file and generates a .meta file based on its internal resources.
        /// @param p_assetPath The path to the asset to be registered. Must be within 
        void RegisterAsset(const std::filesystem::path &p_assetPath);

        /// @brief Loads a resource specified by a guid.
        /// @tparam T The resource type.
        /// @param p_guid The guid of the resource.
        /// @param p_onLoadCallback Callback triggered when the resource's state is ready.
        /// @return A handle to the resource. Make sure to check its validity before use.
        template<typename T>
        ResourceHandle<T> LoadResource(GUID p_guid, std::function<void(ResourceHandle<T>)> p_onLoadCallback = nullptr)
        {
            assert(m_isInitialized && "ResourceManager is not initialized.");

            std::cout << "Manager initialized" << std::endl;
            
            MetaEntry metaEntry;
            if (!m_catalog.GetResourceMetaData(p_guid, metaEntry))
            {
                // Handle missing resource
                // TODO: Log this as a warning/error
                std::cout << "GUID NOT FOUND IN CATALOG" << std::endl;
                return ResourceHandle<T>(C_INVALID_GUID, this);
            }
            std::cout << "Get handle" << std::endl;
            ResourceHandle<T> handle(p_guid, this);
            
            bool loadAsync = false;
            {
                std::lock_guard<std::mutex> lock(m_registryMutex);
                
                auto [it, wasInserted] = m_registry.try_emplace(p_guid);
                if (wasInserted)
                {
                    // Asset was just created in-place -> Update state
                    it->second.state = ResourceState::Queued;
                    it->second.refCount.store(0, std::memory_order_relaxed); // Incremented when handle is constructed
                    loadAsync = true;
                }

                // Handle callback
                if (p_onLoadCallback)
                {
                    if (it->second.state == ResourceState::Ready)
                    {
                        p_onLoadCallback(handle); // Resource is already loaded -> Trigger callback now
                    }
                    else if (it->second.state != ResourceState::Failed)
                    {
                        // Resource is not loaded and has not failed -> Store callback until loaded
                        // Capture handle by value inside lambda to guarantee that ref count is >= 1
                        it->second.loadCallbacks.push_back([p_onLoadCallback, handle]()
                        {
                            p_onLoadCallback(handle);
                        });
                    }
                }
            }
            
            if (loadAsync)
            {
                // TODO: Push load task to worker thread pool
                m_threadPool.PushTask([this, p_guid, metaEntry]()
                {
                    std::cout << "Loading resource on thread: "
                        << std::this_thread::get_id()
                        << std::endl;

                    {
                        std::lock_guard<std::mutex> lock(m_registryMutex);

                        auto it = m_registry.find(p_guid);

                        if (it == m_registry.end())
                        {
                            return;
                        }
                        it->second.state = ResourceState::LoadingAsync;
                    }

                    if constexpr (std::is_same_v<T, TextureResource>)
                    {
                        ResourceLoader::TextureLoader loader;
                        gli::texture texture;
                        texture = loader.Load(metaEntry.path);

                        if (texture.empty())
                        {
                            std::cout << "Texture was not loaded correctly from ResourceManager" << std::endl;
                        }
                        else
                        {
                            std::cout << "Texture was successfully loaded inside ResourceManager" << std::endl;
                        }

                        auto extent = texture.extent();
                        auto mipLevels = texture.levels();

                        auto resource = std::make_unique<Texture2DResource>();
                        resource->SetDimensions(extent.x, extent.y);
                        resource->SetMipLevels(mipLevels);

                        {
                            std::lock_guard<std::mutex> lock(m_registryMutex);

                            auto it = m_registry.find(p_guid);

                            if (it == m_registry.end())
                            {
                                return;
                            }
                            it->second.resource = std::move(resource);
                            it->second.state = ResourceState::ReadyAsync;
                        }
                    }
                });
                    
            }
            
            return handle;
        }

        /// @brief Increments the reference count of a resource in the internal registry.
        /// @param p_guid The globally unique identifier of the resource.
        void IncrementRef(GUID p_guid);

        /// @brief Decrements the reference count of a resource in the internal registry.
        /// @param p_guid The globally unique identifier of the resource.
        void DecrementRef(GUID p_guid);

        /// @brief Queries the load state of a resource in the internal registry.
        /// @param p_guid The globally unique identifier of the resource.
        /// @return The state that the resource is currently in.
        ResourceState GetState(GUID p_guid);

        /// @brief Gets the raw resource stored in the internal registry specified by a GUID.
        /// @tparam T The resource type.
        /// @param p_guid The globally unique identifier of the resource.
        /// @return The resource.
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

        // temporarily function so that load functions can be tested in AssetLoading.cpp
        bool GetCachedMetaDataForAsset(
            const std::string &p_assetPath,
            std::vector<MetaEntry> &p_metaData)
        {
            return m_catalog.GetCachedMetaDataForAsset(p_assetPath, p_metaData);
        }
    
    private:
        /// @brief Compiles a new set of meta entries for an asset based on previously known resources and what entries 
        /// were found in the asset file.
        /// @param p_assetPath The path to the asset file that the resource is stored in.
        /// @param p_foundResources The resources found in the asset file.
        /// @return The updated list of resources that should be listed for the asset.
        std::vector<MetaEntry> CompareAndCompileMetaData(const std::string &p_assetPath, 
            const std::vector<std::pair<ResourceType, std::string>> &p_foundResources);
        
        bool m_isInitialized = false;
        ResourceCatalog m_catalog;
        
        std::unordered_map<GUID, ResourceRecord> m_registry;
        std::mutex m_registryMutex; 
        
        // TODO: Add thread pool / job system

        ThreadPool m_threadPool;
    };
    
}

#include "resource/ResourceHandle.inl" // Included here to avoid circular definitions


