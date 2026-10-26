#pragma once

#include <functional>

#include <vulkan/vulkan.h>

#include <Graphics/VK/RenderTarget.hpp>
#include <Graphics/VK/PresentTarget.hpp>
#include <Graphics/VK/Pipeline.hpp>
#include <Graphics/VK/Allocator.hpp>
#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/DescriptorSet.hpp>
#include <Graphics/VK/Image.hpp>
#include <Graphics/VK/ImageView.hpp>
#include <Graphics/VK/Sampler.hpp>
#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/IndexBuffer.hpp>
#include <Graphics/VK/UniformBuffer.hpp>
#include <Graphics/RenderData.hpp>
#include <Resource/ResourceManager.hpp>

namespace Droplet::Graphics
{
    // Forward declarations
    class Renderer;
    class Context;
}

namespace Droplet::Graphics::VK
{
    // TODO: <REFACTOR>
    // Should probably not be in the Droplet::Graphics::VK namespace without an enum or something
    constexpr std::uint32_t INVALID_INDEX { std::numeric_limits<std::uint32_t>::max() };
    // TODO: </REFACTOR>
    
    struct PipelineKey
    {
        std::uint64_t vsGuid{ 0 };
        std::uint64_t tcsGuid{ 0 };
        std::uint64_t tesGuid{ 0 };
        std::uint64_t gsGuid{ 0 };
        std::uint64_t fsGuid{ 0 };

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
        
        /// @brief Creates Vulkan buffer with the inputted resource type
        /// @param p_data Data to store in the buffer
        /// @param p_type Descriptor type of the buffer resource
        /// @returns Index of the buffer resource
        template <typename T>
        [[nodiscard]] std::uint32_t CreateBufferResource(const T &p_data, vk::DescriptorType p_type);
        
        /// @brief Updates buffer at specified index
        /// @param p_index Index of the buffer
        /// @param p_data Data to update with
        /// @param p_type Descriptor type of the buffer resource
        template <typename T>
        void UpdateBufferResourceAt(std::uint32_t p_index, const T &p_data, vk::DescriptorType p_type);
        
        /// @brief Creates a render target
        /// @param p_width Width of the render target in pixels
        /// @param p_height Height of the render target in pixels
        /// @returns Index to the render target
        [[nodiscard]] std::uint32_t CreateRenderTarget(std::uint32_t p_width, std::uint32_t p_height);
        
        /// @brief Getter function for a render target at a specified index
        /// @param p_index Index of the render target
        /// @returns The render target
        [[nodiscard]] RenderTarget &GetRenderTargetAt(const std::uint32_t p_index) { return m_renderTargets.at(p_index); }
        
        /// @brief Creates a swapchain present target
        /// @param p_presentImage Presentation image of the swapchain
        /// @param p_format Format of the image
        /// @param p_width Width of the presentation image in pixels
        /// @param p_height Height of the presentation image in pixels
        /// @returns Index to present target
        /// @note Should only be used when creating the swapchain
        [[nodiscard]] std::uint32_t CreatePresentTarget(vk::Image p_presentImage, vk::Format p_format, std::uint32_t p_width, std::uint32_t p_height);
        
        /// @brief Getter function for a present target at a specified index
        /// @param p_index Index of the present target
        /// @returns The present target
        [[nodiscard]] PresentTarget &GetPresentTargetAt(const std::uint32_t p_index) { return m_presentTargets.at(p_index); }
        
        /// @brief Creates an ImGui texture based on a render target
        /// @param p_renderTarget The render target to base the texture off of
        /// @returns Index to ImGui Texture
        [[nodiscard]] std::uint32_t CreateImGuiTexture(const RenderTarget &p_renderTarget);
        
        /// @brief Getter function for an ImGui texture at a specified index
        /// @param p_index Index of the ImGui texture
        /// @returns Reference to the texture in the form of a descriptor set
        [[nodiscard]] vk::DescriptorSet &GetImGuiTextureAt(const std::uint32_t p_index) { return m_imGuiTextures.at(p_index); }
        
        /// @brief Recreates render target and ImGui texture and updates them with new params
        /// @param p_targetIndex Index of the render target
        /// @param p_textureIndex Index of the texture
        /// @param p_width Width of the new render target in pixels
        /// @param p_height Height of the new render target in pixels
        void UpdateImGuiRenderTarget(std::uint32_t p_targetIndex, uint32_t p_textureIndex, std::uint32_t p_width, std::uint32_t p_height);
        
    private:
        // Friend class Renderer to give it access to internals
        friend class Renderer;
        
        IntermediaryStorage() = default;
        ~IntermediaryStorage() = default;
        
        /// @brief Initialise the intermediary storage layer
        /// @param p_context Vulkan Context
        void Initialise(const Context &p_context);
        
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
        /// @param p_device Vulkan device
        /// @param p_physicalDevice Vulkan physical device
        /// @param p_surfaceFormat Vulkan surface format
        /// @returns Vulkan Pipeline
        [[nodiscard]] const Pipeline &GetPipeline(RenderData p_renderData, const vk::raii::Device& p_device, const vk::raii::PhysicalDevice& p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat);
        
        // TODO: </REFACTOR>
            
        /// TODO: Write doxygen (Lio)
        [[nodiscard]] const Mesh &GetMesh(RenderData p_renderData);

        /// TODO: Write doxygen (Lio)
        [[nodiscard]] std::uint32_t GetTextureID(RenderData p_renderData, CommandPool &p_commandPool, bool &result);
        
        [[nodiscard]] std::vector<vk::DescriptorSet> GetDescriptorSets() const; 
        
        // Vulkan Context Pointer (non-owning)
        const Context *m_context = nullptr;
        
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

        std::vector<Image> m_imageMap {};
        std::vector<ImageView> m_viewMap {};
        std::vector<RenderTarget> m_renderTargets {};
        std::vector<UniformBuffer> m_uniformBuffers {};
        std::vector<PresentTarget> m_presentTargets {};
        Sampler m_textureSampler {};

        std::map<std::uint64_t, std::uint32_t> m_textureGuidMap {};
        
        // ImGui
        std::vector<vk::DescriptorSet> m_imGuiTextures {};
    };
}

#include "IntermediaryStorage.inl"
