#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <Graphics/Vulkan/Context.hpp>
#include <Graphics/Vulkan/Image.hpp>
#include <Graphics/Vulkan/ImageView.hpp>
#include <Graphics/Vulkan/PresentTarget.hpp>
#include <Graphics/Vulkan/Pipeline.hpp>
#include <Graphics/Vulkan/Allocator.hpp>
#include <Graphics/Vulkan/DescriptorPool.hpp>
#include <Graphics/Vulkan/DescriptorSet.hpp>
#include <Graphics/Vulkan/Sampler.hpp>
#include <Graphics/Vulkan/VertexBuffer.hpp>
#include <Graphics/Vulkan/IndexBuffer.hpp>
#include <Graphics/Vulkan/UniformBuffer.hpp>
#include <Resource/ResourceManager.hpp>

namespace Droplet
{
    // Forward declarations
    template <typename>
    class DynamicResourceHandle;
}

namespace Droplet::Graphics
{
    // Forward declarations
    class Renderer;
    class RenderTarget;
    class ImGuiTexture; 
    struct RenderData;
}

namespace Droplet::Graphics::Vulkan
{
    // TODO: <REFACTOR>
    // Should probably not be in the Droplet::Graphics::VK namespace without an enum or something
    constexpr std::uint32_t C_INVALID_INDEX { std::numeric_limits<std::uint32_t>::max() };
    // TODO: </REFACTOR>
    
    enum class DynamicResourceType : std::uint8_t 
    {
        Buffer = 0,
        Image = 1,
        ImageView = 2,
        ImGuiTexture = 3,
        INVALID = std::numeric_limits<std::uint8_t>::max()
    };
    
    struct DynamicLiveResource
    {
        DynamicResourceType type { DynamicResourceType::INVALID };
        std::uint32_t refCount {};
        std::unique_ptr<IDynamicResource> resource { nullptr };
    };
    
    struct PipelineKey
    {
        std::uint64_t vsGuid{ 0 };
        std::uint64_t tcsGuid{ 0 };
        std::uint64_t tesGuid{ 0 };
        std::uint64_t gsGuid{ 0 };
        std::uint64_t fsGuid{ 0 };
        std::uint8_t transparency { 0 };

        std::strong_ordering operator<=>(const PipelineKey&) const = default;
    };

    struct PipelineHasher
    {
        std::size_t operator()(const PipelineKey& key) const
        {
            std::size_t res = 17;
            res = res * 31 + std::hash<std::uint64_t>()(key.vsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.tcsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.tesGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.gsGuid);
            res = res * 31 + std::hash<std::uint64_t>()(key.fsGuid);
            res = res * 31 + std::hash<std::uint8_t>()(key.transparency);

            return res;

        }
    };

    struct MeshKey
    {
        std::uint64_t meshGUID;

        std::strong_ordering operator<=>(const MeshKey&) const = default;
    };

    struct Mesh
    {
        bool valid = false;
        vk::Buffer vertexBuffer;
        vk::Buffer indexBuffer;
        std::uint32_t indexCount;
    };

    struct MeshHasher
    {
        std::size_t operator()(const MeshKey& key) const
        {
            std::size_t res = 17;
            res = res * 31 + std::hash<std::uint64_t>()(key.meshGUID);

            return res;
        }
    };

    struct TextureKey
    {
        std::uint64_t texGUID;

        std::strong_ordering operator<=>(const TextureKey&) const = default;
    };

    struct TextureHasher
    {
        std::size_t operator()(const TextureKey& key) const
        {
            std::size_t res = 17;
            res = res * 31 + std::hash<std::uint64_t>()(key.texGUID);

            return res;
        }
    };

    struct MeshBuffer
    {
        VertexBuffer vertices{ nullptr };
        IndexBuffer indices{ nullptr };

        MeshBuffer& operator=(MeshBuffer&& p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }

            std::swap(vertices, p_other.vertices);
            std::swap(indices, p_other.indices);

            return *this;
        }

        bool operator==(const MeshBuffer& p_other) const 
        {
            return indices == p_other.indices && vertices == p_other.vertices;
        }

        MeshBuffer(MeshBuffer&& p_other) noexcept
        {
            std::swap(vertices, p_other.vertices);
            std::swap(indices, p_other.indices);
        }
    };

    struct SampleTexture
    {
        Sampler sampler;
        Image image;
        ImageView textureView;
    };

    class IntermediaryStorage
    {
    public:
        /// @brief Getter function for the global Intermediary Storage Instance
        /// @returns IntermediaryStorage Instance
        [[nodiscard]] static IntermediaryStorage &GetInstance()
        {
            static IntermediaryStorage instance {};
            return instance; 
        }
        
        IntermediaryStorage(const IntermediaryStorage &p_other) = delete;
        IntermediaryStorage& operator=(const IntermediaryStorage &p_other) = delete;
        IntermediaryStorage(IntermediaryStorage &&p_other) noexcept = delete;
        IntermediaryStorage& operator=(IntermediaryStorage &&p_other) noexcept = delete;
            
        /// @brief Get a live Vulkan object based on GUID
        /// @param p_guid GUID of Vulkan object
        /// @returns Pointer to Vulkan object
        template <typename T>
        [[nodiscard]] T* GetLiveObject(GUID p_guid) const;
        
        /// @brief Get the current reference count for a given Vulkan object
        /// @param p_guid GUID of Vulkan object
        /// @returns Reference count
        [[nodiscard]] std::uint32_t GetLiveObjectRefCount(GUID p_guid) const;
        
        /// @brief Increment reference count of Vulkan object
        /// @param p_guid GUID of Vulkan object
        void IncrementRef(GUID p_guid);
        
        /// @brief Decrement reference count of Vulkan object
        /// @param p_guid GUID of Vulkan object
        void DecrementRef(GUID p_guid);
        
        /// @brief Queue Vulkan object for destruction
        /// @param p_guid GUID of Vulkan object
        void MoveToDestructionQueue(GUID p_guid);
        
        /// @brief Destroy Vulkan objects in destruction queue when it is safe to do so
        void DestroyUnusedResources();
        
        /// @brief Creates Vulkan buffer with the inputted resource type
        /// @param p_data Data to store in the buffer
        /// @param p_type Descriptor type of the buffer resource
        /// @returns Index of the buffer resource
        template <typename T>
        [[nodiscard]] std::uint32_t CreateBufferResource(const T &p_data,
            vk::DescriptorType p_type);
        
        /// @brief Updates buffer at specified index
        /// @param p_index Index of the buffer
        /// @param p_data Data to update with
        /// @param p_type Descriptor type of the buffer resource
        template <typename T>
        void UpdateBufferResourceAt(std::uint32_t p_index,
            const T &p_data,
            vk::DescriptorType p_type);
        
        /// @brief Creates a render target
        /// @param p_width Width of the render target in pixels
        /// @param p_height Height of the render target in pixels
        /// @returns Index to the render target
        [[nodiscard]] RenderTarget CreateRenderTarget(std::uint32_t p_width, 
            std::uint32_t p_height);
        
        [[nodiscard]] RenderTarget CreateRenderTarget(GUID p_colourImageGuid,
            GUID p_colourImageViewGuid,
            GUID p_depthImageGuid,
            GUID p_depthImageViewGuid, 
            std::uint32_t p_width,
            std::uint32_t p_height);
        
        /// @brief Creates a swapchain present target
        /// @param p_presentImage Presentation image of the swapchain
        /// @param p_format Format of the image
        /// @param p_width Width of the presentation image in pixels
        /// @param p_height Height of the presentation image in pixels
        /// @returns Index to present target
        /// @note Should only be used when creating the swapchain
        [[nodiscard]] std::uint32_t CreatePresentTarget(vk::Image p_presentImage,
            vk::Format p_format,
            std::uint32_t p_width,
            std::uint32_t p_height);
        
        /// @brief Getter function for a present target at a specified index
        /// @param p_index Index of the present target
        /// @returns The present target
        [[nodiscard]] PresentTarget &GetPresentTargetAt(const std::uint32_t p_index) { return m_presentTargets.at(p_index); }
        
        /// @brief Clears present targets
        void ClearPresentTargets() { m_presentTargets.clear(); }

#ifdef _EDITOR
        /// @brief Creates an ImGui texture based on a render target
        /// @param p_renderTarget The render target to base the texture off of
        /// @returns Index to ImGui Texture
        [[nodiscard]] ImGuiTexture CreateImGuiTexture(const RenderTarget &p_renderTarget);
        
        /// @brief Creates ImGuiTexture with GUID from the Resource Manager
        /// @param p_guid GUID to texture resource
        [[nodiscard]] ImGuiTexture CreateImGuiTexture(GUID p_guid);
        
        /// @brief Recreates render target and ImGui texture and updates them with new params
        /// @param p_target The render target to be updated
        /// @param p_texture ImGui texture
        /// @param p_width Width of the new render target in pixels
        /// @param p_height Height of the new render target in pixels
        void UpdateImGuiRenderTarget(RenderTarget p_target, 
            ImGuiTexture p_texture,
            std::uint32_t p_width,
            std::uint32_t p_height);
        
#endif
        
    private:
        // Friend class Renderer to give it access to internals
        friend class Renderer;
        
        IntermediaryStorage() = default;
        ~IntermediaryStorage() = default;
        
        /// @brief Initialise the intermediary storage layer
        /// @param p_context Vulkan Context
        /// @param p_commandPool
        void Initialise(const Context &p_context,
            const CommandPool &p_commandPool);
        
        /// @brief Shutdown and clear all vulkan memory allocations  
        void Shutdown();
        
        /// @brief Getter function for the VMA allocator
        /// @returns VMA allocator
        /// @note Use carefully as misuse will result in memory leaks or crashes
        [[nodiscard]] const vma::raii::Allocator &GetAllocator() const { return m_allocator.Get(); }
        
        // TODO: <REFACTOR>
        // --   Should return vk::raii::Pipeline instead
        //      New function for vk::raii::PipelineLayout needs to be created
        // --   Refactor Get functions to have similar handling of invalid GUIDs
        
        /// @brief Gets (and possibly creates uncached) Vulkan pipelines 
        /// @param p_renderData Render data with shader information
        /// @param p_surfaceFormat Vulkan surface format
        /// @returns Vulkan Pipeline
        [[nodiscard]] Pipeline *GetPipeline(RenderData p_renderData,
                                            vk::SurfaceFormatKHR p_surfaceFormat);
        
        // TODO: </REFACTOR>
            
        /// TODO: Write doxygen (Lio)
        [[nodiscard]] const Mesh GetMesh(const vk::raii::CommandBuffer &p_commandBuffer,
            RenderData p_renderData);

        /// TODO: Write doxygen (Lio)
        [[nodiscard]] std::uint32_t GetTextureID(RenderData p_renderData,
            CommandPool &p_commandPool,
            bool &result);
        
        [[nodiscard]] std::vector<vk::DescriptorSet> GetDescriptorSets() const; 
        
        // Vulkan Context Pointer (non-owning)
        const Context *m_context = nullptr;
        const CommandPool *m_commandPool = nullptr;
        
        // Vulkan thingies
        Allocator m_allocator { nullptr };
        DescriptorPool m_descriptorPool { nullptr };
        std::vector<DescriptorSet> m_descriptorSets {};
        
        std::unordered_map<vk::DescriptorType, std::uint32_t> m_descriptorSetIndexMap 
        {
            { vk::DescriptorType::eUniformBuffer, 0 },
            { vk::DescriptorType::eStorageBuffer, 1 },
            { vk::DescriptorType::eCombinedImageSampler, 2 }
        };
        
        // Stored buffers etc
        std::unordered_map<PipelineKey, Pipeline, PipelineHasher> m_pipelineMap {};
        std::unordered_map<MeshKey, VertexBuffer, MeshHasher> m_vertexMap {};
        std::unordered_map<MeshKey, IndexBuffer, MeshHasher> m_indexMap {};
        std::unordered_map<std::uint32_t, UniformBuffer> m_worldMap {};

        // TODO: Remove?
        std::vector<Image> m_imageMap {};
        std::vector<ImageView> m_viewMap {};
        std::vector<PresentTarget> m_presentTargets {};
        std::vector<UniformBuffer> m_uniformBuffers {};
        
        // TODO: Remove?
        Sampler m_textureSampler {};

        std::map<std::uint64_t, std::uint32_t> m_textureGuidMap {};
 
        // NEW STUFF
        
        std::unordered_map<GUID, DynamicLiveResource> m_liveObjects {};
        std::queue<DynamicLiveResource> m_destroyQueue {};

        
        // DESCRIPTORS
        
        // Types of Descriptor Sets:
        // - Sampler
        // - Combined Image Sampler
        // - Sampled Image
        // - Uniform Texel Buffer
        // - Storage Texel Buffer
        // - Uniform Buffer
        // - Storage Buffer
        // - Uniform Buffer Dynamic
        // - Storage Buffer Dynamic
        // - Input Attachment
    
    };
}

#include "IntermediaryStorage.inl"
