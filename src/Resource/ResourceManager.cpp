#include "resource/ResourceManager.hpp"

#include "Core/StringUtils.hpp"
#include "resource/meta/MetaUtils.hpp"
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet
{
    void ResourceManager::Initialize(const std::filesystem::path &p_rootDirectory)
    {
        ZoneScoped;

        if (m_isInitialized)
        {
            // TODO: (LOGGING) Log warning: RM already initialized
            return;
        }
        

        try
        {
            m_registry.ScanDirectory(p_rootDirectory);
        }
        catch (...)
        {
            // TODO: (LOGGING) Log error: Directory does not exist
            m_isInitialized = false;
            return;
        }
        
        m_isInitialized = true;
        m_rootDirectory = p_rootDirectory;
    }

    void ResourceManager::Update()
    {
		ZoneScopedC(tracy::Color::PaleGreen1);

        assert(m_isInitialized && "Resource manager is not initialized.");
        
        // User callbacks are collected under the lock and invoked after it is released.
        std::vector<std::function<void()>> deferredCallbacks;
#ifdef _EDITOR
        std::vector<GUID> editorChanged;
#endif
        std::unique_lock<std::recursive_mutex> lock(m_mutex);

        // A possible performance improvement for this function could be to limit the number of processed load and register
        // operations per call.
        
        // --- Process Register Results ---
        AsyncRegisterResult regRes;
        while (m_asyncRegisterResults.Pop(regRes))
        {
            // Compare found resources with ones already registered for this asset (if there are any) and determine
            // which are old (keep), which are new (add) and which have been erased (remove).
            std::vector<MetaEntry> newMetaData = CompareAndCompileMetaData(regRes.relAssetPath, regRes.foundResources);
                
            // Update internal catalog
            for (const auto &entry : newMetaData)
            {
                m_registry.RegisterMetaEntry(regRes.relAssetPath.generic_string(), entry);
            }
            
            if (regRes.onCompleteCallback)
            {
                deferredCallbacks.emplace_back([callback = regRes.onCompleteCallback, succeeded = regRes.succeeded, newMetaData]()
                {
                    callback(succeeded, newMetaData);
                });
            }
                
            // Push metafile write operation to worker thread
            ThreadPool::GetInstance().PushTask([metaPath = std::move(regRes.absoluteMetaPath), metaData = std::move(newMetaData)]()
            {
               MetaUtils::Write(metaPath, metaData); 
            });
        }
        
        // --- Process Load Results ---
        AsyncLoadResult loadRes;
        while (m_asyncLoadResults.Pop(loadRes))
        {
            void* resourcePtr = loadRes.resource;
            auto soundIt = m_pendingSoundLoads.find(loadRes.guid);
            if (soundIt != m_pendingSoundLoads.end())
            {
                resourcePtr = soundIt->second;
                m_pendingSoundLoads.erase(soundIt);
            }
            
            auto it = m_liveResources.find(loadRes.guid);
            if (it == m_liveResources.end())
            {
                // Resource was destroyed before load finished
                delete static_cast<IResource*>(resourcePtr); // Release the allocated resource
                continue;
            }
            
            LiveResource &liveResource = it->second;
            
            if (!loadRes.succeeded)
            {
                liveResource.state = ResourceState::Failed;
                delete static_cast<IResource*>(resourcePtr);
                
                // Trigger callbacks
                std::vector<std::function<void()>> callbacksToInvoke;
                callbacksToInvoke.swap(liveResource.loadCallbacks);
                for (auto &callback : callbacksToInvoke)
                {
                    deferredCallbacks.push_back(std::move(callback));
                }
                
                continue;
            }

            liveResource.resource.reset(static_cast<IStaticResource*>(resourcePtr));
            liveResource.state = ResourceState::Ready;
            
            std::vector<std::function<void()>> callbacksToInvoke;
            callbacksToInvoke.swap(liveResource.loadCallbacks);
            
            // Invoke all callbacks registered for the resource
            for (auto &callback : callbacksToInvoke)
            {
                deferredCallbacks.push_back(std::move(callback));
            }
        }

#ifdef _EDITOR
        const auto now = std::chrono::steady_clock::now();
        if (now - m_editorLastFileCheck >= std::chrono::seconds(1))
        {
            m_editorLastFileCheck = now;

            for (const auto &[guid, liveResource] : m_liveResources)
            {
                MetaEntry metaEntry;
                if (!m_registry.GetResourceMetaData(guid, metaEntry))
                {
                    continue;
                }

                const std::filesystem::path assetPath = m_rootDirectory / metaEntry.relAssetPath;
                std::error_code errorCode;
                const std::filesystem::file_time_type writeTime =
                    std::filesystem::last_write_time(assetPath, errorCode);
                if (errorCode)
                {
                    continue;
                }

                auto knownTime = m_editorFileTimes.find(guid);
                if (knownTime == m_editorFileTimes.end())
                {
                    m_editorFileTimes.emplace(guid, writeTime);
                    continue;
                }

                if (knownTime->second != writeTime)
                {
                    knownTime->second = writeTime;
                    editorChanged.push_back(guid);
                }
            }
        }
#endif

        lock.unlock();

        for (auto &callback : deferredCallbacks)
        {
            callback();
        }

#ifdef _EDITOR
        for (const GUID guid : editorChanged)
        {
            ReloadResource(guid);
        }
#endif
    }

    std::filesystem::path ResourceManager::GetRootDirectory() const
    {
        return m_rootDirectory;
    }

    void ResourceManager::RegisterAsset(const std::filesystem::path &p_relAssetPath, 
        std::function<void(bool p_succeeded, const std::vector<MetaEntry>&)> p_onComplete)
    {
        ZoneScoped;
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::filesystem::path relAssetPath(p_relAssetPath);
        if (relAssetPath.is_absolute())
        {
            // TODO: (LOGGING) Log error here
            return;
        }
        
        // Calculate absolute path for loaders
        std::filesystem::path absAssetPath = m_rootDirectory / relAssetPath;
        std::filesystem::path absMetaPath = absAssetPath.generic_string() + ".meta";
        
        std::string ext = absAssetPath.extension().generic_string();
        StringUtils::ToLowerInPlace(ext);
        
        auto it = m_assetScanners.find(ext);
        if (it == m_assetScanners.end())
        {
            // No Scanner was found
            if (p_onComplete) p_onComplete(false, {});
            return;
        }
        AssetScannerFunc scannerFunc = it->second;
        
        ThreadPool::GetInstance().PushTask([this, absAssetPath, relAssetPath, absMetaPath, scannerFunc, p_onComplete](){
            // --- Async Worker Thread ---
            AsyncRegisterResult res;
            res.relAssetPath = relAssetPath; // Relative for registry
            res.absoluteMetaPath = absMetaPath; // Absolute for meta write
            res.onCompleteCallback = p_onComplete;

            try
            {
                res.foundResources = scannerFunc(absAssetPath); // Scan the asset for resources
                res.succeeded = true;
            }
            catch (...)
            {
                res.succeeded = false;
            }
            
            m_asyncRegisterResults.Push(res);
        });
    }
    
    std::vector<const MetaEntry *> ResourceManager::GetRegisteredResources(StaticResourceType p_type) const
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_registry.GetEntries(p_type);
    }

    bool ResourceManager::GetResourceMetaEntry(const GUID p_guid, MetaEntry &p_metaEntry) const
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_registry.GetResourceMetaData(p_guid, p_metaEntry);
    }
    
    bool ResourceManager::GetResourceMetaEntry(const std::string &p_resourceName, MetaEntry &p_metaEntry) const
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        return m_registry.GetResourceMetaData(p_resourceName, p_metaEntry);
    }

    std::vector<MetaLoadSettings::Descriptor> ResourceManager::GetSettingsSchema(StaticResourceType p_type) const
    {
        return MetaUtils::GetSettingsSchema(p_type);
    }

    bool ResourceManager::UpdateMetaEntry(GUID p_guid, const MetaEntry &p_updatedEntry, bool hotReload)
    {
        ZoneScoped;

        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::unique_lock<std::recursive_mutex> lock(m_mutex);

        // Safety check to not change meta while a worker thread could be reading from it
        ResourceState state = GetState(p_guid);
        if (state == ResourceState::LoadingAsync || state == ResourceState::Queued || state == ResourceState::Uploading)
        {
            // TODO: (LOGGING) Log warning
            return false;
        }
        
        // Sanitize entry
        MetaEntry originalEntry;
        if (!m_registry.GetResourceMetaData(p_guid, originalEntry))
        {
            return false;
        }
        
        MetaEntry sanitizedEntry = p_updatedEntry;
        sanitizedEntry.guid = originalEntry.guid;
        sanitizedEntry.type = originalEntry.type;
        sanitizedEntry.relAssetPath = originalEntry.relAssetPath;
        
        // Fetch meta list from asset
        std::string relAssetPathStr = sanitizedEntry.relAssetPath;
        std::vector<MetaEntry> assetMetaData;
        if (!m_registry.GetCachedMetaDataForAsset(relAssetPathStr, assetMetaData))
        {
            // TODO: (LOGGING) Log asset not being registered
            return false;
        }
        
        // Replace old entry in our vector
        for (auto &entry : assetMetaData)
        {
            if (entry.guid == p_guid)
            {
                entry = sanitizedEntry;
                break;
            }
        }
        m_registry.RegisterMetaEntry(relAssetPathStr, sanitizedEntry);
        
        // Push an overwrite task to a worker thread
        std::filesystem::path absMetaPath = m_rootDirectory / (relAssetPathStr + ".meta");
        ThreadPool::GetInstance().PushTask([metaPath = absMetaPath, metaData = std::move(assetMetaData)]()
        {
           MetaUtils::Write(metaPath, metaData); 
        });
        
        lock.unlock();
        if (hotReload)
        {
            ReloadResource(p_guid);
        }
        
        return true;
    }

    void ResourceManager::IncrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            it->second.refCount += 1;
        }
    }

    void ResourceManager::DecrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
        
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            if (--it->second.refCount == 0)
            {
                if (it->second.type == StaticResourceType::Sound)
                {
                    Audio::AudioSystem::GetInstance().UnloadSound(reinterpret_cast<SoundResource*>(it->second.resource.get()));
                }
                
                m_liveResources.erase(it);
#ifdef _EDITOR
                m_editorFileTimes.erase(p_guid);
#endif
            }
        }
    }

    std::uint32_t ResourceManager::GetRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        std::lock_guard<std::recursive_mutex> lock(m_mutex);

        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            return it->second.refCount;
        }
        else
        {
            return C_INVALID_GUID;
        }
    }

    ResourceState ResourceManager::GetState(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
            
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            return it->second.state;
        }
            
        return ResourceState::Unloaded;
    }

    void ResourceManager::RegisterAssetScanner(const std::vector<std::string> &p_extensions,
        AssetScannerFunc p_scanFunc)
    {
        for (const std::string &ext : p_extensions)
        {
            std::string lowerExt = StringUtils::ToLower(ext);
            m_assetScanners[lowerExt] = p_scanFunc;
        }
    }

    bool ResourceManager::IsAssetSupported(const std::filesystem::path &p_path)
    {
        if (!p_path.has_extension())
        {
            return false;
        }
        
        std::string ext = p_path.extension().generic_string();
        StringUtils::ToLowerInPlace(ext);
        
        return m_assetScanners.contains(ext);
    }

    void ResourceManager::ReloadResource(GUID p_guid)
    {
        ZoneScoped;

        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        std::vector<std::function<void(GUID)>> reloadCallbacks;
        auto notifyReload = [&]()
        {
            for (const auto &callback : reloadCallbacks)
            {
                callback(p_guid);
            }
        };
        
        auto it = m_liveResources.find(p_guid);
        if (it == m_liveResources.end())
        {
            return; // Not loaded
        }
        
        LiveResource &live = it->second;
        
        // Avoid ongoing loads/uploads
        if (live.state == ResourceState::LoadingAsync || live.state == ResourceState::Queued || live.state == ResourceState::Uploading)
        {
            return;
        }
        
        live.resource.reset(); // Clear RAM
        live.state = ResourceState::LoadingAsync;

        reloadCallbacks = m_reloadCallbacks;
        
        MetaEntry metaEntry; 
        if (!m_registry.GetResourceMetaData(p_guid, metaEntry))
        {
            // TODO: Log error (resource with p_guid was not found)
            live.state = ResourceState::Failed;
            lock.unlock();
            notifyReload();
            return;
        }

#ifdef _EDITOR
        std::error_code errorCode;
        const std::filesystem::file_time_type writeTime =
            std::filesystem::last_write_time(m_rootDirectory / metaEntry.relAssetPath, errorCode);
        if (!errorCode)
        {
            m_editorFileTimes[p_guid] = writeTime;
        }
#endif
        
        // Look up the load dispatcher for the resource type and call it
        auto dispatcherIt = m_loadDispatchers.find(metaEntry.type);
        if (dispatcherIt != m_loadDispatchers.end())
        {
            dispatcherIt->second(p_guid, metaEntry);
        }
        else
        {
            // TODO: Log error (type is not registered)
            live.state = ResourceState::Failed;
        }

        lock.unlock();
        notifyReload();
    }

    void ResourceManager::RegisterReloadCallback(std::function<void(GUID)> p_onReload)
    {
        if (p_onReload)
        {
            m_reloadCallbacks.push_back(std::move(p_onReload));
        }
    }

    void ResourceManager::RemoveResourceMeta(const GUID p_guid, bool p_forceRemove)
    {
        std::lock_guard<std::recursive_mutex> lock(m_mutex);
#ifdef _EDITOR
        m_editorFileTimes.erase(p_guid);
#endif

        if (!p_forceRemove)
        {
            if (GetRef(p_guid) > 0)
            {
                throw std::runtime_error("Cannot remove resource because it is in use");
            }
        }
        
        MetaEntry metaEntry;
        m_registry.GetResourceMetaData(p_guid, metaEntry);
        
        const std::filesystem::path path = m_rootDirectory / (metaEntry.relAssetPath + ".meta");
        if (!std::filesystem::remove(path))
        {
            Debug::Log(Debug::LogType::Warning, "Could not remove resource " + path.generic_string());
            return;
        }
        m_registry.RemoveMetaEntry(p_guid);
        m_liveResources.erase(p_guid);
    }

    std::vector<MetaEntry> ResourceManager::CompareAndCompileMetaData(const std::filesystem::path &p_relAssetPath,
                                                                      const std::vector<std::pair<StaticResourceType, std::string>> &p_foundResources)
    {
        ZoneScoped;

        std::vector<MetaEntry> oldMetaData;
        m_registry.GetCachedMetaDataForAsset(p_relAssetPath.generic_string(), oldMetaData); // If it fails, old metadata remains empty
        
        std::vector<MetaEntry> out;
        out.reserve(p_foundResources.size());
        
        // Iterate over the resources that were found in the asset
        for (const auto& [foundType, foundName] : p_foundResources)
        {
            // Try to find an existing match in the old metadata
            auto it = std::find_if(oldMetaData.begin(), oldMetaData.end(),
                [&](const MetaEntry &existingEntry)
                {
                    return existingEntry.type == foundType && existingEntry.sourceName == foundName;
                }
            );
            
            if (it != oldMetaData.end())
            {
                // Resource already exists in the old metadata -> copy it to the new metadata
                out.push_back(*it);
            }
            else
            {
                // Resource does not exist in the old metadata -> create a new entry for it
                out.push_back(MetaUtils::GenerateDefaultMetaEntry(foundType, foundName, p_relAssetPath.generic_string()));
            }
        }
        
        return out;
    }
}
