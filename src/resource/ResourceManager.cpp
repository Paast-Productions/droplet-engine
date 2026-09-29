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
        
        // Perform all queued tasks
        std::function<void()> task;
        while (m_mainThreadTasks.Pop(task))
        {
            task();
        }
    }

    void ResourceManager::RegisterAsset(const std::filesystem::path &p_assetPath)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::string assetPathStr = p_assetPath.generic_string();
        std::filesystem::path metaPath = assetPathStr + ".meta";
        
        std::string ext = p_assetPath.extension().generic_string();
        StringUtils::ToLowerInPlace(ext);
        

        m_threadPool.PushTask([this, p_assetPath, assetPathStr, metaPath, ext](){
            // --- Async Worker Thread ---
            std::vector<std::pair<ResourceType, std::string>> foundResources;
            
            if (ext == ".fbx" || ext == ".gltf" || ext == ".obj")
            {
                foundResources = AssimpLoader::ListAssetResources(p_assetPath);
            }
            else if (ext == ".png" || ext == ".jpg" || ext == ".ktx" || ext == ".dds")
            {
                foundResources = GliLoader::ListAssetResources(p_assetPath);
            }
            else if (ext == ".slang")
            {
                foundResources = SlangLoader::ListAssetResources(p_assetPath);
            }
            
            m_mainThreadTasks.Push([this, assetPathStr, metaPath, foundResources]()
            {
                // --- Main Thread ---
                
                // Compare found resources with ones already registered for this asset (if there are any) and determine
                // which are old (keep), which are new (add) and which have been erased (remove).
                std::vector<MetaEntry> newMetaData = CompareAndCompileMetaData(assetPathStr, foundResources);
                
                // Update internal catalog
                for (const auto &entry : newMetaData)
                {
                    m_catalog.RegisterMetaEntry(assetPathStr, entry);
                }
                
                m_threadPool.PushTask([metaPath, metaData = std::move(newMetaData)]()
                {
                   // --- Async Worker Thread --- 
                   MetaUtils::Write(metaPath, metaData); 
                });
            });
        });
    }

    void ResourceManager::IncrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            it->second.refCount.fetch_add(1); // TODO: Look into which memory_order to use here
        }
    }

    void ResourceManager::DecrementRef(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex);
        
        auto it = m_registry.find(p_guid);
        if (it != m_registry.end())
        {
            if (it->second.refCount.fetch_sub(1) == 1) // TODO: Look into which memory_order to use here
            {
                m_registry.erase(it);
            }
        }
    }

    ResourceState ResourceManager::GetState(GUID p_guid)
    {
        assert(m_isInitialized && "Resource manager is not initialized.");
        
        std::lock_guard<std::mutex> lock(m_registryMutex); // This is not ideal for polling (performance)
            
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
