#pragma once

#include <cstdint>

namespace engine
{
    class AssetManager;
    
    using GUID = uint64_t;
    
    template<typename T>
    class AssetHandle
    {
    public:
        AssetHandle() = default;
        AssetHandle(GUID p_guid, AssetManager* p_assetManager)
            : m_guid(p_guid), m_assetManager(p_assetManager) {}
        
        ~AssetHandle()
        {
            ReleaseRef();
        }
        
        AssetHandle(const AssetHandle& other) 
            : m_guid(other.m_guid), m_assetManager(other.m_assetManager)
        {
            // Copy construct (+1 ref)
            AddRef();
        }
        
        AssetHandle& operator=(const AssetHandle& other)
        {
            // Copy assign (new: +1 old: -1)
            if (this != &other)
            {
                ReleaseRef();
                m_guid = other.m_guid;
                m_assetManager = other.m_assetManager;
                AddRef();
            }
            
            return *this;
        }
        
        AssetHandle(AssetHandle&& other) noexcept
            : m_guid(other.m_guid), m_assetManager(other.m_assetManager) 
        {
            // Move construct (steal data, no ref change)
            other.m_guid = 0;
            other.m_assetManager = nullptr;
        }
        
        AssetHandle& operator=(AssetHandle&& other) noexcept
        {
            // Move assign (steal to new, no ref change)
            if (this != &other)
            {
                ReleaseRef();
                m_guid = other.m_guid;
                m_assetManager = other.m_assetManager;
                
                other.m_guid = 0;
                other.m_assetManager = nullptr;
            }
            
            return *this;   
        }
        
        /// @return True if the handle is valid, otherwise false.
        bool IsValid() const
        {
            return m_guid != 0 && m_assetManager;
        }

        /// @return The globally unique identifier for the asset the handle is referencing.
        GUID GetGUID() const
        {
            return m_guid;
        }
        
        /// @return The loaded asset stored within the central asset manager, or nullptr if the load failed.
        T* Get() const
        {
            if (IsValid())
            {
                return m_assetManager->GetResource<T>(m_guid);
            }
            
            return nullptr;
        }
        
    private:
        GUID m_guid = 0;
        AssetManager* m_assetManager = nullptr;

        /// @brief Increments the reference of the asset the handle is referencing.
        void AddRef()
        {
            if (IsValid())
            {
                m_assetManager->AddRef(m_guid);
            }
        }

        /// @brief Decrements the reference count of the asset the handle is referencing.
        void ReleaseRef()
        {
            if (IsValid())
            {
                m_assetManager->ReleaseRef(m_guid);
            }
        }
    };
    
}
