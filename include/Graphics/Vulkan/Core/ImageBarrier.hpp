/**
 * @file ImageBarrier.hpp
 * @brief Contains functions to transition images and set up memory barriers.
 */
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Abstraction/Image.hpp"
#include "Vulkan/Core/ImageState.hpp"

namespace Droplet::Graphics::Vulkan
{
    /// @brief Records an image barrier from one state to the next.
    /// @param cmd command buffer to record the barrier to
    /// @param image vk handle to the image
    /// @param state the current state of the image
    /// @param next the state after the transition
    /// @param range the subresource range
    /// @note For work outside the render graph (uploads); graph passes get their barriers from the graph.
    inline void TransitionImage(
        vk::CommandBuffer cmd,
        vk::Image image,
        ImageState& state,
        const ImageState& next,
        const vk::ImageSubresourceRange& range)
    {
        const vk::ImageMemoryBarrier2 barrier = 
        {
            .srcStageMask = state.stageMask,
            .srcAccessMask = state.accessMask,
            .dstStageMask = next.stageMask,
            .dstAccessMask = next.accessMask,
            .oldLayout = state.layout,
            .newLayout = next.layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = range
        };

        const vk::DependencyInfo dependency{
            .imageMemoryBarrierCount = 1,
            .pImageMemoryBarriers = &barrier
        };

        cmd.pipelineBarrier2(dependency);
        state = next;
    }

    /// @brief The same, tracking the state in the image itself (Image::State).
    inline void TransitionImage(
        vk::CommandBuffer commandBuffer,
        Image& image,
        const ImageState& newState,
        vk::ImageAspectFlags aspectMask)
    {
        TransitionImage(
            commandBuffer,
            image.Get(),
            image.State(),
            newState,
            image.GetSubresourceRange(aspectMask));
    }
}
