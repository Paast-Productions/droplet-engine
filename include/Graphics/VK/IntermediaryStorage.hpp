#pragma once

#include <functional>

#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/Pipeline.hpp>
#include <Graphics/VK/Allocator.hpp>
#include <Graphics/RenderData.hpp>
#include <Resource/ResourceManager.hpp>
#include <Graphics/VK/DescriptorSet.hpp>
#include <Graphics/VK/Image.hpp>
#include <Graphics/VK/ImageView.hpp>
#include <Graphics/VK/Sampler.hpp>
#include <Graphics/VK/VertexBuffer.hpp>
#include <Graphics/VK/IndexBuffer.hpp>
#include <Graphics/VK/UniformBuffer.hpp>
#include <vulkan/vulkan.h>

namespace Droplet::Graphics::VK
{
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

        bool operator==(MeshBuffer& p_other) noexcept
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
        IntermediaryStorage() = delete;

        IntermediaryStorage(Context& p_context, DescriptorSet& p_descriptorSet);

        IntermediaryStorage(const IntermediaryStorage& p_other) = delete;
        IntermediaryStorage& operator=(const IntermediaryStorage& p_other) = delete;
        IntermediaryStorage(IntermediaryStorage&& p_other) noexcept = delete;
        IntermediaryStorage& operator=(IntermediaryStorage&& p_other) noexcept = delete;

        ~IntermediaryStorage() = default;

        [[nodiscard]] const vma::raii::Allocator& GetAllocator() const { return m_allocator.Get(); }

        [[nodiscard]] const Pipeline& GetPipeline(RenderData p_renderData, const vk::raii::Device& p_device, const vk::raii::PhysicalDevice& p_physicalDevice, vk::SurfaceFormatKHR p_surfaceFormat);

        [[nodiscard]] const Mesh& GetMesh(RenderData p_renderData);

        const std::uint32_t GetTextureID(RenderData p_renderData, CommandPool &p_commandPool);
        
    private:
        const VK::Context &m_context;
        DescriptorSet &m_descriptorSet;

        VK::Allocator m_allocator { nullptr };
        std::unordered_map<PipelineKey, Pipeline, PipelineHasher> m_pipelineMap {};

        std::unordered_map<MeshKey, VertexBuffer, MeshHasher> m_vertexMap {};
        std::unordered_map<MeshKey, IndexBuffer, MeshHasher> m_indexMap {};

        std::unordered_map<std::uint32_t, UniformBuffer> m_worldMap{};

        std::vector<Image> m_imageMap {};
        std::vector<ImageView> m_viewMap{};
        Sampler m_textureSampler {};

        std::map<std::uint64_t, std::uint32_t> m_textureGuidMap{};
    };
}

