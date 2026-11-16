/**
 * @file FormatUtils.hpp
 * @brief contains utilities for vk::Format for various tasks
 */
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <stdexcept>

namespace Droplet::Utils::Vulkan
{
    /// The aspects an image of `format` has: depth and/or stencil for depth-stencil formats, colour for the rest.
    // Use it for image view aspect masks and barrier subresource ranges.
    constexpr vk::ImageAspectFlags ImageAspectsForFormat(vk::Format format)
    {
        switch (format)
        {
        case vk::Format::eD16Unorm:
        case vk::Format::eX8D24UnormPack32:
        case vk::Format::eD32Sfloat:
            return vk::ImageAspectFlagBits::eDepth;
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        case vk::Format::eS8Uint:
            return vk::ImageAspectFlagBits::eStencil;
        default:
            return vk::ImageAspectFlagBits::eColor;
        }
    }

    constexpr bool FormatHasStencil(vk::Format format) noexcept
    {
        return static_cast<bool>(ImageAspectsForFormat(format) & vk::ImageAspectFlagBits::eStencil);
    }

    /// First of D32, X8D24, D16 the device can use as a depth attachment. Formats with stencil are not considered: the
    /// scene pass never attaches stencil, and a stencil format would then have to be declared (and attached) as such.
    /// @throws std::runtime_error if none is supported (cannot happen on a conformant device: D16 is mandatory).
    inline vk::Format ChooseDepthFormat(const vk::raii::PhysicalDevice& physicalDevice)
    {
        for (const vk::Format candidate :
        {vk::Format::eD32Sfloat, vk::Format::eX8D24UnormPack32, vk::Format::eD16Unorm})
        {
            const vk::FormatProperties props = physicalDevice.getFormatProperties(candidate);
            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
                return candidate;
        }
        throw std::runtime_error("No supported depth format");
    }
}
