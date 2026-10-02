#pragma once

#include <string>
#include <memory>
#include <atomic>
#include <mutex>
#include <filesystem>
#include <cassert>
#include <functional>

#include "resource/loaders/ResourceLoaderTraits.hpp"
#include "resource/ResourceHandle.hpp"
#include "resource/ResourceRegistry.hpp"

#include "core/ThreadSafeQueue.hpp"
#include "core/ThreadPool.hpp"

namespace Droplet
{
    /// @brief Represents all states that a resource can have.
    enum class ResourceState
    {
        Unloaded,
        Queued,
        LoadingAsync,
        Uploading,
        Ready,
        Failed
    };

    /// @brief Contains relevant information about a specific loaded resource.
    struct LiveResource
    {
        std::unique_ptr<IResource> resource = nullptr;
        ResourceState state = ResourceState::Unloaded;
        uint32_t refCount = 0;
        
        std::vector<std::function<void()>> loadCallbacks; // Called when resource hits state: Ready
    };

    /// @brief The package sent back from a worker thread after a load operation has been completed.
    struct AsyncLoadResult
    {
        bool succeeded = true;
        
        GUID guid;
        IResource* resource;
        ResourceLoadFlag loadFlags;
    };

    /// @brief The package sent back from a worker thread after a register operation has been completed.
    struct AsyncRegisterResult
    {
        std::filesystem::path relAssetPath;
        std::filesystem::path absoluteMetaPath;
        std::vector<std::pair<ResourceType, std::string>> foundResources;
    };

    /// @brief Universal resource manager class.
    class ResourceManager
    {
    public:
        ResourceManager() = default;
        ~ResourceManager() = default;

        /// @brief Initializes the resource manager for a specific root directory.
        /// @param p_rootDirectory The absolute path to the root directory containing all assets that the manager should 
        /// be able to load resources from.
        void Initialize(const std::filesystem::path &p_rootDirectory);

        /// @brief Should be called every frame. Processes the internal task queues.
        void Update();

        /// @brief Parses an asset file and generates a .meta file based on its internal resources (or updates an existing one).
        /// @param p_relAssetPath The path to the asset relative to the resource manager's root directory.
        void RegisterAsset(const std::filesystem::path &p_relAssetPath);

        /// @brief Loads a resource specified by a guid.
        /// @tparam T The resource type.
        /// @param p_guid The guid of the resource.
        /// @param p_onLoadCallback Callback triggered (on main thread) when the resource's state is ready.
        /// @return A handle to the resource. Make sure to check its validity before use.
        template<typename T>
        ResourceHandle<T> LoadResource(GUID p_guid, std::function<void(ResourceHandle<T>)> p_onLoadCallback = nullptr)
        {
            static_assert(std::is_base_of_v<IResource, T>, "T must inherit from IResource");
            assert(m_isInitialized && "ResourceManager is not initialized.");

            MetaEntry metaEntry;
            try
            {
                metaEntry = m_registry.GetResourceMetaData(p_guid);
            }
            catch (...)
            {
                // Handle missing resource
                // TODO: Log this as a warning/error
                return ResourceHandle<T>(C_INVALID_GUID, this);
            }
            
            bool loadAsync = false;
            auto [it, wasInserted] = m_liveResources.try_emplace(p_guid);
            if (wasInserted)
            {
                // Resource was just created in-place -> Update its state
                it->second.state = ResourceState::LoadingAsync;
                it->second.refCount = 0; // Incremented when handle is constructed
                loadAsync = true;
            }
            
            ResourceHandle<T> handle(p_guid, this);

            // Handle callback
            if (p_onLoadCallback)
            {
                if (it->second.state == ResourceState::Ready)
                {
                    p_onLoadCallback(handle);
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
            
            if (loadAsync)
            {
                ThreadPool::GetInstance().PushTask([this, p_guid, metaEntry]()
                {
                    // --- Async Worker Thread ---
                    std::filesystem::path absAssetPath = m_rootDirectory / metaEntry.relAssetPath;
                    std::unique_ptr<T> loadedResource = ResourceLoaderTraits<T>::LoadCPU(absAssetPath, metaEntry.loadSettings);
                    
                    AsyncLoadResult res;
                    res.succeeded = (loadedResource != nullptr);
                    res.guid = p_guid;
                    res.resource = loadedResource.release();
                    res.loadFlags = metaEntry.loadFlags;
                    
                    m_asyncLoadResults.Push(res);
                });
            }
            
            return handle;
        }

        /// @brief Retrieves a list of registered resources, optionally filtered by type.
        /// @param p_type The resource type to filter by. Default is None (this returns all resource types).
        /// @return A vector of pointers to the registered metadata entries.
        [[nodiscard]] std::vector<const MetaEntry *> GetRegisteredResources(ResourceType p_type=ResourceType::None) const;

        /// @brief Increments the reference count of a resource in the internal cache.
        /// @param p_guid The globally unique identifier of the resource.
        void IncrementRef(GUID p_guid);

        /// @brief Decrements the reference count of a resource in the internal cache.
        /// @param p_guid The globally unique identifier of the resource.
        void DecrementRef(GUID p_guid);

        /// @brief Retrieves the current reference count for a resource.
        /// @param p_guid The guid of the resource.
        /// @return The reference count.
        std::uint32_t GetRef(GUID p_guid);

        /// @brief Queries the load state of a resource in the internal cache.
        /// @param p_guid The globally unique identifier of the resource.
        /// @return The state that the resource is currently in.
        ResourceState GetState(GUID p_guid);

        /// @brief Gets the raw resource stored in the internal cache specified by a GUID.
        /// @tparam T The resource type.
        /// @param p_guid The globally unique identifier of the resource.
        /// @return The resource.
        template<typename T>
        T *GetResource(GUID p_guid)
        {
            assert(m_isInitialized && "AssetManager is not initialized.");
            
            auto it = m_liveResources.find(p_guid);
            if (it != m_liveResources.end() && it->second.state == ResourceState::Ready)
            {
                // Asset is ready to be used
                return static_cast<T*>(it->second.resource.get());
            }
            
            return nullptr;
        }
    
    private:
        /// @brief Compiles a new set of meta entries for an asset based on previously known resources and what entries 
        /// were found in the asset file.
        /// @param p_relAssetPath The path to the asset file that the resource is stored in.
        /// @param p_foundResources The resources found in the asset file.
        /// @return The updated list of resources that should be listed for the asset.
        std::vector<MetaEntry> CompareAndCompileMetaData(const std::filesystem::path &p_relAssetPath, 
            const std::vector<std::pair<ResourceType, std::string>> &p_foundResources);
        
        bool m_isInitialized = false;
        std::filesystem::path m_rootDirectory; // Absolute path to the root directory
        ResourceRegistry m_registry;
        std::unordered_map<GUID, LiveResource> m_liveResources;
        
        ThreadSafeQueue<AsyncRegisterResult> m_asyncRegisterResults;
        ThreadSafeQueue<AsyncLoadResult> m_asyncLoadResults;
    };
    
}

#include "resource/ResourceHandle.inl" // Included here to avoid circular definitions


