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
            // Log RM already initialized
            return;
        }
        
        if (!m_catalog.ScanDirectory(p_rootDirectory))
        {
            // Log error (directory does not exist)
            m_isInitialized = false;
            return;
        }
        
        m_rootDirectory = std::filesystem::absolute(p_rootDirectory);
        m_isInitialized = true;
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
            std::vector<MetaEntry> newMetaData = CompareAndCompileMetaData(regRes.assetPathStr, regRes.foundResources);
                
            // Update internal catalog
            for (const auto &entry : newMetaData)
            {
                m_catalog.RegisterMetaEntry(regRes.assetPathStr, entry);
            }
                
            // Push metafile write operation to worker thread
            ThreadPool::GetInstance().PushTask([metaPath = std::move(regRes.metaPath), metaData = std::move(newMetaData)]()
            {
               MetaUtils::Write(metaPath, metaData); 
            });
        }
        
        // --- Process Load Results ---
        AsyncLoadResult loadRes;
        while (m_asyncLoadResults.Pop(loadRes))
        {
            auto it = m_registry.find(loadRes.guid);
            if (it == m_registry.end())
            {
                // Resource was destroyed before load finished
                delete loadRes.resource; // Release the allocated resource stored in the load result
                continue;
            }
            
            ResourceRecord &record = it->second;
            
            if (!loadRes.succeeded)
            {
                record.state = ResourceState::Failed;
                delete loadRes.resource; // Safety delete (resource should already be nullptr)
                continue;
            }
            record.resource.reset(loadRes.resource);
            
            switch (loadRes.loadFlags)
            {
                case ResourceLoadFlag::LoadCPU:
                {
                    record.state = ResourceState::Ready;
                    break;
                }
                case ResourceLoadFlag::LoadGPU:
                {
                    record.state = ResourceState::Uploading;
                                        
                    // TODO: Implement upload to GPU upload queue or upload here directly
                    
                    record.resource.reset(); // Release resource from RAM
                    record.state = ResourceState::Ready;
                    break;
                }
                case ResourceLoadFlag::LoadBoth:
                {
                    record.state = ResourceState::Uploading;
                                        
                    // TODO: Implement upload to GPU upload queue or upload here directly
                                        
                    record.state = ResourceState::Ready;
                    break;
                }
            }
            
            std::vector<std::function<void()>> callbacksToInvoke;
            callbacksToInvoke.swap(record.loadCallbacks);
            
            // Invoke all callbacks registered for the resource
            for (auto &callback : callbacksToInvoke)
            {
                callback();
            }
        }
    }

    void ResourceManager::RegisterAsset(const std::filesystem::path &p_assetPath)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::string assetPathStr = p_assetPath.generic_string();
        std::filesystem::path metaPath = assetPathStr + ".meta";
        
        std::string ext = p_assetPath.extension().generic_string();
        StringUtils::ToLowerInPlace(ext);
        
        ThreadPool::GetInstance().PushTask([this, p_assetPath, assetPathStr, metaPath, ext](){
            // --- Async Worker Thread ---
            AsyncRegisterResult res;
            res.assetPathStr = std::move(assetPathStr);
            res.metaPath = std::move(metaPath);
            
            if (ext == ".fbx" || ext == ".gltf" || ext == ".obj")
            {
                res.foundResources = AssimpLoader::ListAssetResources(p_assetPath);
            }
            else if (ext == ".png" || ext == ".jpg" || ext == ".ktx" || ext == ".dds")
            {
                res.foundResources = GliLoader::ListAssetResources(p_assetPath);
            }
            else if (ext == ".slang")
            {
                res.foundResources = SlangLoader::ListAssetResources(p_assetPath);
            }
            
            m_asyncRegisterResults.Push(res);
        });
    }

    void ResourceManager::IncrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1); // TODO: Look into which memory_order to use here
        }
    }

    void ResourceManager::DecrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            if (it->second.refCount.fetch_sub(1) == 1) // TODO: Look into which memory_order to use here
            {
                m_registry.erase(it);
            }
        }
    }

    uint32_t ResourceManager::GetRef(GUID p_guid)
    {
        assert(m_isInitialized && "AssetManager is not initialized.");

        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
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
            
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            return it->second.state;
        }
            
        return ResourceState::Unloaded;
    }

    std::vector<MetaEntry> ResourceManager::CompareAndCompileMetaData(const std::string &p_assetPath,
        const std::vector<std::pair<ResourceType, std::string>> &p_foundResources)
    {
        std::vector<MetaEntry> oldMetaData;
        m_catalog.GetCachedMetaDataForAsset(p_assetPath, oldMetaData); // If it fails, old metadata remains empty
        
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
                out.push_back(MetaUtils::GenerateDefaultMetaEntry(foundType, foundName, p_assetPath));
            }
        }
        
        return out;
    }
}
