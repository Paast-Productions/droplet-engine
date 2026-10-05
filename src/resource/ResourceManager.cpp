#include "resource/ResourceManager.hpp"

#include "core/StringUtils.hpp"
#include "resource/meta/MetaUtils.hpp"

#include "resource/loaders/AssimpLoader.hpp"
#include "resource/loaders/GliLoader.hpp"
#include "resource/loaders/SlangLoader.hpp"

namespace Droplet
{
    void ResourceManager::Initialize(const std::filesystem::path &p_rootDirectory)
    {
        if (m_isInitialized)
        {
            // TODO: Log warning: RM already initialized
            return;
        }
        

        try
        {
            m_registry.ScanDirectory(p_rootDirectory);
        }
        catch (...)
        {
            // TODO: Log error: Directory does not exist
            m_isInitialized = false;
            return;
        }
        
        m_isInitialized = true;
        m_rootDirectory = p_rootDirectory;
    }

    void ResourceManager::Update()
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
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
            auto it = m_liveResources.find(loadRes.guid);
            if (it == m_liveResources.end())
            {
                // Resource was destroyed before load finished
                delete loadRes.resource; // Release the allocated resource stored in the load result
                continue;
            }
            
            LiveResource &liveResource = it->second;
            
            if (!loadRes.succeeded)
            {
                liveResource.state = ResourceState::Failed;
                delete loadRes.resource; // Safety delete (resource should already be nullptr)
                
                // Trigger callbacks
                std::vector<std::function<void()>> callbacksToInvoke;
                callbacksToInvoke.swap(liveResource.loadCallbacks);
                for (auto &callback : callbacksToInvoke)
                {
                    callback();
                }
                
                continue;
            }
            liveResource.resource.reset(loadRes.resource);
            
            switch (loadRes.loadFlags)
            {
                case ResourceLoadFlag::LoadCPU:
                {
                    liveResource.state = ResourceState::Ready;
                    break;
                }
                case ResourceLoadFlag::LoadGPU:
                {
                    liveResource.state = ResourceState::Uploading;
                                        
                    // TODO: Implement upload to GPU upload queue or upload here directly
                    
                    liveResource.resource.reset(); // Release resource from RAM
                    liveResource.state = ResourceState::Ready;
                    break;
                }
                case ResourceLoadFlag::LoadBoth:
                {
                    liveResource.state = ResourceState::Uploading;
                                        
                    // TODO: Implement upload to GPU upload queue or upload here directly
                                        
                    liveResource.state = ResourceState::Ready;
                    break;
                }
            }
            
            std::vector<std::function<void()>> callbacksToInvoke;
            callbacksToInvoke.swap(liveResource.loadCallbacks);
            
            // Invoke all callbacks registered for the resource
            for (auto &callback : callbacksToInvoke)
            {
                callback();
            }
        }
    }

    void ResourceManager::RegisterAsset(const std::filesystem::path &p_relAssetPath)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::filesystem::path relAssetPath(p_relAssetPath);
        if (relAssetPath.is_absolute())
        {
            // TODO: Log error here
            return;
        }
        
        // Calculate absolute path for loaders
        std::filesystem::path absAssetPath = m_rootDirectory / relAssetPath;
        std::filesystem::path absMetaPath = absAssetPath.generic_string() + ".meta";
        
        std::string ext = absAssetPath.extension().generic_string();
        StringUtils::ToLowerInPlace(ext);
        
        ThreadPool::GetInstance().PushTask([this, absAssetPath, relAssetPath, absMetaPath, ext](){
            // --- Async Worker Thread ---
            AsyncRegisterResult res;
            res.relAssetPath = relAssetPath; // Relative for registry
            res.absoluteMetaPath = absMetaPath; // Absolute for meta write
            
            if (ext == ".fbx" || ext == ".glb" || ext == ".gltf" || ext == ".obj")
            {
                res.foundResources = AssimpLoader::ListAssetResources(absAssetPath);
            }
            else if (ext == ".png" || ext == ".jpg" || ext == ".ktx" || ext == ".dds")
            {
                res.foundResources = GliLoader::ListAssetResources(absAssetPath);
            }
            else if (ext == ".slang")
            {
                res.foundResources = SlangLoader::ListAssetResources(absAssetPath);
            }
            
            m_asyncRegisterResults.Push(res);
        });
    }

    
    std::vector<const MetaEntry *> ResourceManager::GetRegisteredResources(ResourceType p_type) const
    {
        return m_registry.GetEntries(p_type);
    }

    void ResourceManager::IncrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            it->second.refCount += 1;
        }
    }

    void ResourceManager::DecrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            if (--it->second.refCount == 0)
            {
                m_liveResources.erase(it);
            }
        }
    }

    std::uint32_t ResourceManager::GetRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");

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
            
        auto it = m_liveResources.find(p_guid);
        if (it != m_liveResources.end())
        {
            return it->second.state;
        }
            
        return ResourceState::Unloaded;
    }

    std::vector<MetaEntry> ResourceManager::CompareAndCompileMetaData(const std::filesystem::path &p_relAssetPath,
        const std::vector<std::pair<ResourceType, std::string>> &p_foundResources)
    {
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
                    return existingEntry.type == foundType && existingEntry.name == foundName;
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
