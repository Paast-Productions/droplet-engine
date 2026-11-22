/**
 * @file GraphicsPipeline.hpp
 * @brief Pipeline state description (plain data, hashable) and the pipeline object built from it.
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <vector>

#include "Integration/Core/Guid.hpp" // assumed: Guid is equality-comparable and has a std::hash specialization

namespace Vulkan
{
    class Shader;

    struct VertexLayout
    {
        std::vector<vk::VertexInputBindingDescription> bindings;
        std::vector<vk::VertexInputAttributeDescription> attributes;

        friend bool operator==(const VertexLayout&, const VertexLayout&) = default;
    };

    struct RasterState
    {
        vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
        vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
        vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

        friend bool operator==(const RasterState&, const RasterState&) = default;
    };

    struct DepthState
    {
        bool testEnable = false;
        bool writeEnable = false;
        vk::CompareOp compareOp = vk::CompareOp::eLess;

        friend bool operator==(const DepthState&, const DepthState&) = default;
    };

    enum class BlendMode : std::uint8_t
    {
        Opaque,
        AlphaBlend,
        Additive
    };

    /**
     * @brief Everything that defines a graphics pipeline. No Vulkan objects are owned here.
     *
     * Shaders are referenced by GUID so the description can be serialized / stored on a material.
     * Attachment formats are part of the description because dynamic-rendering pipelines are
     * format-specific: a swapchain format change simply produces a different key.
     */
    struct GraphicsPipelineDesc
    {
        Droplet::GUID vertexShader{};
        Droplet::GUID fragmentShader{};   ///< Leave default (null) for depth-only pipelines
        bool hasFragmentShader = true;

        vk::PipelineLayout layout{};      ///< Non-owning; layouts live longer than pipelines

        VertexLayout vertexLayout{};      ///< Empty for fullscreen-triangle passes
        vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
        RasterState raster{};
        DepthState depth{};

        std::vector<vk::Format> colorFormats;
        std::vector<BlendMode> colorBlend; ///< One per color format
        vk::Format depthFormat = vk::Format::eUndefined;
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;

        /// Hash of the description (shader GUIDs included); PipelineLibrary's cache key.
        [[nodiscard]] std::uint64_t Hash() const;

        /// Full comparison; PipelineLibrary checks it on a hash hit.
        friend bool operator==(const GraphicsPipelineDesc&, const GraphicsPipelineDesc&) = default;
    };

    /**
     * @brief A built graphics pipeline (the PSO). Viewport and scissor are always dynamic.
     */
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(
            const vk::raii::Device& device,
            const GraphicsPipelineDesc& desc,
            const Shader& vertex,
            const Shader* fragment,
            const vk::raii::PipelineCache* cache = nullptr);

        [[nodiscard]] vk::Pipeline Handle() const { return *m_pipeline; }
        [[nodiscard]] vk::PipelineLayout Layout() const { return m_layout; }

        void Bind(vk::CommandBuffer cmd) const
        {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_pipeline);
        }

    private:
        vk::raii::Pipeline m_pipeline{nullptr};
        vk::PipelineLayout m_layout{};
    };
}
