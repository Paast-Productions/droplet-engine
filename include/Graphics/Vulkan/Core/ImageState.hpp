/**
 * @file ImageState.hpp
 * @brief Defines ImageState is a struct that assists in tracking the transitional state of images, and helper functions
 to create it for common states.
*/
#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
    /// @brief struct used to track the state of an Image in Transition
    struct ImageState
    {
        vk::ImageLayout layout;
        vk::AccessFlags2 accessMask;
        vk::PipelineStageFlags2 stageMask;

        /// Creates an image state for an undefined attachment
        static constexpr ImageState Undefined()
        {
            return {
                .layout = vk::ImageLayout::eUndefined,
                .accessMask = {},
                .stageMask = {}
            };
        }

        /**
         * @brief Creates an image state for a present attachment
         * @details As a barrier's destination: no access (present is not a pipeline access), but the stage must not be
         * empty. The submit's semaphore signal (render-finished, stage ALL_COMMANDS) only waits for the layout
         * transition if the barrier's destination stage chains with it; with eNone, present could see the
         * image before the transition (SYNC-HAZARD-PRESENT-AFTER-WRITE with synchronisation validation).
         */
        static constexpr ImageState Present()
        {
            return {
                .layout = vk::ImageLayout::ePresentSrcKHR,
                .accessMask = {},
                .stageMask = vk::PipelineStageFlagBits2::eAllCommands
            };
        }

        /// Creates an image state for a transfer destination attachment
        static constexpr ImageState TransferDst()
        {
            return {
                .layout = vk::ImageLayout::eTransferDstOptimal,
                .accessMask = vk::AccessFlagBits2::eTransferWrite,
                .stageMask = vk::PipelineStageFlagBits2::eTransfer
            };
        }

        /// Creates an image state for a transfer source attachment
        static constexpr ImageState TransferSrc()
        {
            return {
                .layout = vk::ImageLayout::eTransferSrcOptimal,
                .accessMask = vk::AccessFlagBits2::eTransferRead,
                .stageMask = vk::PipelineStageFlagBits2::eTransfer
            };
        }

        /// Creates an image state for a color attachment
        static constexpr ImageState ColorAttachment()
        {
            return {
                .layout = vk::ImageLayout::eColorAttachmentOptimal,
                .accessMask =
                vk::AccessFlagBits2::eColorAttachmentRead |
                vk::AccessFlagBits2::eColorAttachmentWrite,
                .stageMask =
                vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };
        }

        /// Creates an image state for a depth attachment
        static constexpr ImageState DepthAttachment()
        {
            return {
                .layout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
                .accessMask =
                vk::AccessFlagBits2::eDepthStencilAttachmentRead |
                vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                .stageMask =
                vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                vk::PipelineStageFlagBits2::eLateFragmentTests
            };
        }

        /// Creates an image state for a sampled image defaulting to the fragment shader stage
        static constexpr ImageState Sampled(
            vk::PipelineStageFlags2 shaderStages = vk::PipelineStageFlagBits2::eFragmentShader)
        {
            return {
                .layout = vk::ImageLayout::eShaderReadOnlyOptimal,
                .accessMask = vk::AccessFlagBits2::eShaderSampledRead,
                .stageMask = shaderStages
            };
        }
    };
}
