/**
 * @file UiPass.hpp
 * @brief Vulkan::UiPass: draws the frame's UI (Render::UiDrawList: fills and images) on top of the composite.
 */
#pragma once

#include <cstdint>
#include <span>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "Render/PreparedFrame.hpp"
#include "Vulkan/Abstraction/Buffer.hpp"

namespace Vulkan
{
    class Context;
    class GpuResourceManager;

    /// @brief UI rectangles, alpha blended: resolved to pixels for the target's size, written into this frame slot's
    ///        vertex buffer, drawn in order (back to front), one draw per run of rectangles with the same image.
    ///        One pipeline per colour format; descriptor sets from a pool per frame slot.
    class UiPass
    {
    public:
        /// @brief Distinct images one frame's UI can show (the white texture of plain fills is one of them)
        static constexpr std::uint32_t MAX_IMAGES_PER_FRAME = 64;

        /// @param context        must outlive the pass
        /// @param gpu            where the rectangles' image handles are looked up; must outlive the pass
        /// @param framesInFlight one vertex buffer and one descriptor pool per frame slot
        UiPass(const Context& context, const GpuResourceManager& gpu, std::uint32_t framesInFlight);

        // no copy no move
        UiPass(const UiPass&) = delete;
        UiPass& operator=(const UiPass&) = delete;
        UiPass(UiPass&&) = delete;
        UiPass& operator=(UiPass&&) = delete;

        /// @brief Sets the shaders (ui.slang); pipelines are rebuilt on next use. @pre the GPU is not using them
        void CreateShaderModules(const std::vector<std::uint32_t>& vertexSpirv,
                                 const std::vector<std::uint32_t>& fragmentSpirv);

        /// @brief Starts a frame in `frameSlot`: resets the slot's descriptor pool.
        /// @pre the GPU is done with the slot's previous frame
        void BeginFrame(std::uint32_t frameSlot);

        /**
         * @brief Draws `rects` (UiDrawList::items, from Vulkan::Prepare) into the open render pass instance (the
         *        composite's), covering `extent` pixels.
         * @pre inside a render pass instance with one colour attachment of `colorFormat`; called at most once per
         *      frame; the render targets among the rectangles' images are in ShaderReadOnly (the graph's SampledRead)
         */
        void Record(vk::CommandBuffer commandBuffer, std::span<const Render::UiDrawItem> rects, vk::Extent2D extent,
                    vk::Format colorFormat);

    private:
        /// @brief What the vertex shader reads
        struct Vertex
        {
            glm::vec2 position; ///< pixels, (0,0) top-left
            glm::vec2 uv;       ///< in the image, (0,0) top-left
            glm::vec4 color;    ///< as the target stores it (linear for an sRGB target), straight alpha
        };

        /// @brief What a descriptor set binds
        struct SampledImage
        {
            vk::ImageView view;
            vk::Sampler sampler;
            friend bool operator==(const SampledImage&, const SampledImage&) = default;
        };

        /// @brief A descriptor set of this frame and the image it binds
        struct ImageSet
        {
            SampledImage image;
            vk::DescriptorSet set;
        };

        /// @brief Consecutive rectangles with the same image: one draw
        struct Batch
        {
            vk::DescriptorSet set;
            std::uint32_t firstVertex{ 0 };
            std::uint32_t vertexCount{ 0 };
        };

        /// @brief The view and sampler `binding` stands for; the placeholder texture if its handle is stale
        [[nodiscard]] SampledImage ImageFor(const Render::ImageBinding& binding) const;

        /// @brief This frame's set for `image`, allocated from the slot's pool the first time; null when the frame
        ///        already shows MAX_IMAGES_PER_FRAME images
        [[nodiscard]] vk::DescriptorSet SetFor(const SampledImage& image);

        /// @brief The pipeline rendering to `colorFormat`, created the first time
        [[nodiscard]] vk::Pipeline PipelineFor(vk::Format colorFormat);

        const Context* m_context;        ///< Not owned; outlives this.
        const GpuResourceManager* m_gpu; ///< the device's; not owned
        vk::raii::ShaderModule m_vertexShaderModule{ nullptr };
        vk::raii::ShaderModule m_fragmentShaderModule{ nullptr };
        vk::raii::DescriptorSetLayout m_setLayout{ nullptr }; ///< set 0 binding 0: the rectangle's image
        vk::raii::PipelineLayout m_pipelineLayout{ nullptr };
        std::unordered_map<vk::Format, vk::raii::Pipeline> m_pipelines; ///< by colour attachment format

        std::vector<vk::raii::DescriptorPool> m_framePools; ///< one per frame slot, reset when the slot is reused
        std::vector<ImageSet> m_frameSets;   ///< this frame's sets, one per distinct image
        std::vector<Buffer> m_vertexBuffers; ///< one per frame slot, CPU-written (MemoryAccess::Dynamic), grown on need
        std::vector<Vertex> m_vertices;      ///< this frame's vertices, reused between frames
        std::vector<Batch> m_batches;        ///< this frame's draws, reused between frames
        std::uint32_t m_frameSlot{ 0 };
        bool m_reportedTooManyImages{ false };
    };
}
