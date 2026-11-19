#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <stdexcept>
#include <string_view>
#include <string>

namespace Vulkan::Utils
{
    /**
     * Throws a runtime error with the given reason and vk::Result as string
     * @param result vk::Result code
     * @param reason string with reason for failure
     * @throws std::runtime_error with reason + result as string
     */
    [[noreturn]] inline
    void ThrowResult(vk::Result result, std::string_view reason)
    {
        throw std::runtime_error(std::string(reason) + "\n  Result: " + vk::to_string(result));
    }

    /**
     * Helper function to throw if vk::Result is not one of the expected results
     * @tparam ExpectedResults acceptable vk::Result values.
     * @param result result to check
     * @param reason reason to throw if the result is not valid
     * @param expectedResults acceptable results, if left empty will check for vk::Result::eSuccess
     */
    template <typename... ExpectedResults>
        requires (std::same_as<ExpectedResults, vk::Result> && ...)
    void CheckResult(
        vk::Result result,
        std::string_view reason,
        ExpectedResults... expectedResults)
    {
        // if no expected results, only allow success
        if constexpr (sizeof...(expectedResults) == 0)
        {
            if (result == vk::Result::eSuccess)
                return;
        }
        else if (((result == expectedResults) || ...))
        {
            return;
        };

        ThrowResult(result, reason);
    }

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
