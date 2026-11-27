/**
 * @file GraphicsPipeline.hpp
 * @brief Pipeline state description (plain data, hashable) and the pipeline object built from it.
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <vector>

#include "Core/Guid.hpp" // assumed: Guid is equality-comparable and has a std::hash specialization

namespace Droplet::Graphics::Vulkan
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
        float lineWidth = 1.0f; ///< above 1 only on a device with wide lines

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
     * @brief The shader stages a graphics pipeline is built from
     * @details A stage is used when its GUID is valid; leave the others out:
     *          `ShaderGroup{ .vertex = vs, .geometry = gs, .fragment = fs }`
     */
    struct ShaderGroup
    {
        GUID vertex{ C_INVALID_GUID };
        GUID tessControl{ C_INVALID_GUID };
        GUID tessEvaluation{ C_INVALID_GUID };
        GUID geometry{ C_INVALID_GUID };
        GUID fragment{ C_INVALID_GUID }; ///< invalid: depth only (no colour written)

        /// @brief True if the group has both tessellation stages
        [[nodiscard]] bool HasTessellation() const noexcept
        {
            return tessControl != C_INVALID_GUID && tessEvaluation != C_INVALID_GUID;
        }

        /// @brief True if the group has a geometry stage
        [[nodiscard]] bool HasGeometry() const noexcept { return geometry != C_INVALID_GUID; }

        /**
         * @brief True if the group can draw a scene object: a vertex and a fragment stage, and tessellation stages
         *        either both or neither (Vulkan needs the pair)
         */
        [[nodiscard]] bool IsValid() const noexcept
        {
            const bool tessellationPaired = (tessControl == C_INVALID_GUID) == (tessEvaluation == C_INVALID_GUID);
            return vertex != C_INVALID_GUID && fragment != C_INVALID_GUID && tessellationPaired;
        }

        friend bool operator==(const ShaderGroup&, const ShaderGroup&) = default;
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
        ShaderGroup shaders{};            ///< a stage is used when its GUID is valid; no fragment: depth only

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
        /// @brief The created shader stages a pipeline is built from; null: the stage is not used
        struct Stages
        {
            const Shader* vertex = nullptr; ///< required
            const Shader* tessControl = nullptr;
            const Shader* tessEvaluation = nullptr;
            const Shader* geometry = nullptr;
            const Shader* fragment = nullptr;
        };

        /**
         * @brief Builds the pipeline
         * @param device The device
         * @param desc The state; desc.shaders names the stages `stages` holds
         * @param stages The created stages, each with its stage's type
         * @param cache Optional pipeline cache
         * @throws std::invalid_argument if a stage is missing or of the wrong type, or the description is inconsistent
         */
        GraphicsPipeline(
            const vk::raii::Device& device,
            const GraphicsPipelineDesc& desc,
            const Stages& stages,
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
