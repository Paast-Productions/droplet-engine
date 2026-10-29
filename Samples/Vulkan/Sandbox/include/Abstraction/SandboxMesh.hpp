#pragma once

#include <typeindex>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Sandbox::Resources
{
    class ResourceManager;
    
    // Resource handle
    template<typename T>
    class ResourceHandle {
    private:
        std::string resourceId;
        ResourceManager* resourceManager;

    public:
        ResourceHandle() : resourceManager(nullptr) {}

        ResourceHandle(const std::string& id, ResourceManager* manager)
            : resourceId(id), resourceManager(manager) {}

        T* Get() const {
            if (!resourceManager) return nullptr;
            return resourceManager->GetResource<T>(resourceId);
        }

        bool IsValid() const {
            return resourceManager && resourceManager->HasResource<T>(resourceId);
        }

        const std::string& GetId() const {
            return resourceId;
        }

        // Convenience operators
        T* operator->() const {
            return Get();
        }

        T& operator*() const {
            return *Get();
        }

        operator bool() const {
            return IsValid();
        }
    };
    
    class Resource
    {
    public:
        explicit Resource(const std::string&id) : resourceId(id){}
        virtual ~Resource() = default;
        
        const std::string &GetId()const {return resourceId;}
        bool IsLoaded()const{return loaded; }
        
        // virtual interface
        bool Load()
        {
            loaded = doLoad();
            return loaded;
        }
        
        void Unload()
        {
            doUnload();
            loaded = false;
        }
    protected:
        virtual bool doLoad() = 0;
        virtual bool doUnload() = 0;
    private:
        std::string resourceId;
        bool loaded = false;
    };
    
    class ResourceManager {
    public:
    private:
        std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<Resource>>> resources;
        
        struct ResourceData
        {
            std::shared_ptr<Resource> resource;
            int refCount;
        };
        std::unordered_map<std::type_index, std::unordered_map<std::string, ResourceData>> refCounts;
        
    };
    
    class Mesh : public Resource
    {
    public:
        explicit Mesh(const std::string &id)
        {
            
        }
        ~Mesh()
        {
            Unload();
        }
        
        void Unload() override {
            // Only proceed with cleanup if resources are currently loaded
            if (IsLoaded()) {
                // Phase 3a: Obtain device handle for resource destruction
                vk::Device device = GetDevice();

                // Phase 3b: Destroy buffers and free GPU memory in proper sequence
                // Index resources cleaned up first to maintain clear dependency order
                device.destroyBuffer(indexBuffer);         // Destroy index buffer object
                device.freeMemory(indexBufferMemory);      // Release index buffer memory

                // Vertex resources cleaned up second
                device.destroyBuffer(vertexBuffer);        // Destroy vertex buffer object
                device.freeMemory(vertexBufferMemory);     // Release vertex buffer memory

                // Phase 3c: Update base class state to reflect unloaded condition
                Resource::Unload();
            }
        }

        // Public interface for safe access to GPU resources and metadata
        vk::Buffer GetVertexBuffer() const { return vertexBuffer; }
        vk::Buffer GetIndexBuffer() const { return indexBuffer; }
        uint32_t GetVertexCount() const { return vertexCount; }
        uint32_t GetIndexCount() const { return indexCount; }
        
    private:
        // Vertex data management - stores per-vertex attributes like position, normal, UV coordinates
        vk::Buffer vertexBuffer;                // GPU buffer containing vertex attribute data
        vk::DeviceMemory vertexBufferMemory;    // GPU memory backing the vertex buffer
        vk::DeviceSize vertexBufferOffset;      // Offset within the memory allocation for vertex buffer
        uint32_t vertexCount = 0;               // Number of vertices in this mesh

        // Index data management - defines triangle connectivity using vertex indices
        vk::Buffer indexBuffer;                 // GPU buffer containing triangle index data
        vk::DeviceMemory indexBufferMemory;     // GPU memory backing the index buffer
        vk::DeviceSize indexBufferOffset;       // Offset within the memory allocation for index buffer
        uint32_t indexCount = 0;                // Number of indices in this mesh (typically 3 per triangle)
    };
}