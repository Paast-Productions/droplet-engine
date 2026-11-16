/**
 * @file Compositor.hpp
 * @brief Vulkan::Compositor: the fullscreen pipelines the render graph's fragment-effect, resolve and composite
 *        passes record with.
 */
#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Vulkan/Passes/PostProcessShader.hpp"

namespace Vulkan
{
    class Context;

    /// @brief Fullscreen triangle that samples one image and applies a PostProcess::Effect. One pipeline per target
    ///        format, built on first use; one descriptor pool per frame slot, reset by BeginFrame.
    class Compositor
    {
    public:
        static constexpr std::uint32_t MAX_FULLSCREEN_PASSES_PER_FRAME{32}; ///< sets per frame slot's pool

        /// @brief Creates the sampler, layouts and one descriptor pool per frame slot.
        /// @param context must outlive the compositor
        Compositor(const Context& context, std::uint32_t framesInFlight);
        ~Compositor() = default;

        Compositor(const Compositor&) = delete;
        Compositor& operator=(const Compositor&) = delete;
        Compositor(Compositor&&) = delete;
        Compositor& operator=(Compositor&&) = delete;

        /// @brief Sets the fullscreen vertex shader ("vMain") and the post-process fragment shader ("fMain").
        /// @pre the GPU is not using the pipelines (they are rebuilt on next use)
        void CreateShaderModules(
            const std::vector<std::uint32_t>& vertexSpirv,
            const std::vector<std::uint32_t>& fragmentSpirv);

        /// @brief Resets the slot's descriptor pool. @pre the GPU has finished the slot's previous frame
        void BeginFrame(std::uint32_t frameSlot);

        /// @brief Records one fullscreen pass: samples `source` (ShaderReadOnlyOptimal) into all of `destination`
        ///        (ColorAttachmentOptimal). A null `source` only clears.
        /// @param destinationFormat selects the pipeline
        /// @param overlay           recorded after the fullscreen draw, inside the same render pass instance (the UI)
        /// @throws vk::SystemError if the frame's pool is exhausted (more than MAX_FULLSCREEN_PASSES_PER_FRAME passes)
        void RecordFullscreen(
            vk::CommandBuffer commandBuffer,
            vk::ImageView source,
            vk::ImageView destination,
            vk::Format destinationFormat,
            vk::Extent2D extent,
            const PostProcess::PushConstants& pushConstants,
            const std::function<void(vk::CommandBuffer)>& overlay = {});

    private:
        void CreateDescriptorSetLayout();
        void CreateSampler();
        void CreateDescriptorPools();
        void CreatePipelineLayout();

        /// @brief The pipeline rendering to `colorFormat`, created the first time.
        [[nodiscard]] vk::Pipeline PipelineFor(vk::Format colorFormat);

        /// @brief Allocates a set from the current slot's pool and points it at `source`.
        [[nodiscard]] vk::DescriptorSet AllocateSourceSet(vk::ImageView source);

        const vk::raii::Device& Device() const noexcept;

    private:
        const Context* m_context; ///< Not owned; outlives the compositor.
        std::uint32_t m_framesInFlight;
        std::uint32_t m_frameSlot{0}; ///< Slot of the frame being recorded (BeginFrame)

        vk::ClearValue m_defaultClearColor{vk::ClearColorValue{{{0.01f, 0.01f, 0.1f, 1.0f}}}};

        // == Pipelines ==
        vk::raii::ShaderModule m_vertexShaderModule{nullptr};
        vk::raii::ShaderModule m_fragmentShaderModule{nullptr};
        vk::raii::DescriptorSetLayout m_descriptorSetLayout{nullptr};
        vk::raii::Sampler m_sampler{nullptr};
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};
        std::unordered_map<vk::Format, vk::raii::Pipeline> m_pipelines; ///< by colour attachment format

        // == Descriptors ==
        std::vector<vk::raii::DescriptorPool> m_framePools; ///< one per frame slot; reset whole, never freed per set
    };
}
