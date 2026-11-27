/**
 * @file EffectPass.hpp
 * @brief Vulkan::EffectPass: the compute pipelines the render graph's compute-effect passes record with.
 */
#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Passes/PostProcessShader.hpp"

namespace Droplet::Graphics::Vulkan
{
    class Context;

    /// @brief Compute shaders of the post-process effects: sample one image, apply a PostProcess::Effect and write
    ///        a storage image of the same size and/or the effect's step buffer. One pipeline per entry point, one
    ///        descriptor pool per frame slot, reset by BeginFrame.
    class EffectPass
    {
    public:
        static constexpr std::uint32_t MAX_EFFECT_PASSES_PER_FRAME{32}; ///< sets per frame slot's pool
        static constexpr std::uint32_t WORK_GROUP_SIZE{8}; ///< matches numthreads in Shaders/postprocess.cs.slang

        /// Format of the images a compute effect writes; matches the shader's storage image. sRGB formats cannot
        /// be storage images, so a compute step writes RGBA16F and a later graphics pass writes the view's target.
        static constexpr vk::Format IMAGE_FORMAT{vk::Format::eR16G16B16A16Sfloat};

        /// @brief Creates the sampler, layouts and one descriptor pool per frame slot.
        /// @param context must outlive the pass
        EffectPass(const Context& context, std::uint32_t framesInFlight);
        ~EffectPass() = default;

        EffectPass(const EffectPass&) = delete;
        EffectPass& operator=(const EffectPass&) = delete;
        EffectPass(EffectPass&&) = delete;
        EffectPass& operator=(EffectPass&&) = delete;

        /// @brief SPIR-V of the post-process compute shader's entry points.
        struct ShaderCode
        {
            std::vector<std::uint32_t> imageEffect;      ///< "cMain": image to image
            std::vector<std::uint32_t> measureLuminance; ///< "cMeasureLuminance": image to step buffer
            std::vector<std::uint32_t> applyExposure;    ///< "cApplyExposure": image and step buffer to image
        };

        /// @brief Builds one pipeline per entry point.
        /// @pre the GPU is not using the pipelines
        void CreatePipelines(const ShaderCode& shaderCode);

        /// @brief Resets the slot's descriptor pool. @pre the GPU has finished the slot's previous frame
        void BeginFrame(std::uint32_t frameSlot);

        /// @brief Records one effect step: samples `source` (ShaderReadOnlyOptimal) and writes every pixel of
        ///        `destination` (General, IMAGE_FORMAT) and/or the step buffer, as the step's effect needs.
        /// @param destination null for a step that only writes the step buffer
        /// @param stepBuffer  null for a step that does not use it
        /// @param extent      size of `destination`
        /// @throws vk::SystemError if the frame's pool is exhausted (more than MAX_EFFECT_PASSES_PER_FRAME passes)
        void Record(
            vk::CommandBuffer commandBuffer,
            vk::ImageView source,
            vk::ImageView destination,
            vk::Buffer stepBuffer,
            vk::Extent2D extent,
            const PostProcess::PushConstants& pushConstants);

    private:
        void CreateDescriptorSetLayout();
        void CreateSampler();
        void CreateDescriptorPools();
        void CreatePipelineLayout();

        /// @brief Allocates a set from the current slot's pool and points it at the step's images and buffer; a
        ///        null one is left unwritten (its entry point does not use it).
        [[nodiscard]] vk::DescriptorSet AllocateStepSet(
            vk::ImageView source, vk::ImageView destination, vk::Buffer stepBuffer);

        const vk::raii::Device& Device() const noexcept;

    private:
        const Context* m_context; ///< Not owned; outlives the pass.
        std::uint32_t m_framesInFlight;
        std::uint32_t m_frameSlot{0}; ///< Slot of the frame being recorded (BeginFrame)

        // == Pipeline ==
        vk::raii::DescriptorSetLayout m_descriptorSetLayout{nullptr};
        vk::raii::Sampler m_sampler{nullptr};
        vk::raii::PipelineLayout m_pipelineLayout{nullptr};
        vk::raii::Pipeline m_imageEffectPipeline{nullptr};
        vk::raii::Pipeline m_measureLuminancePipeline{nullptr};
        vk::raii::Pipeline m_applyExposurePipeline{nullptr};

        // == Descriptors ==
        std::vector<vk::raii::DescriptorPool> m_framePools; ///< one per frame slot; reset whole, never freed per set
    };
}
