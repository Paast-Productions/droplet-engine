#pragma once

#include <Resource/GuidUtils.hpp>

#include <glm/glm.hpp>

#include <cstdint>

namespace Droplet::Graphics::Common
{
     // == Images and render targets ==

    /// @brief agnostic version of pixel formats
    enum class PixelFormat
    {
        RGBA8_SRGB,
        RGBA8_UNORM
    };

    struct Extent2D{
        std::uint32_t width{0};
        std::uint32_t height{0};
    };

    struct Rect{
        glm::vec2 position{0.0f};
        glm::vec2 size{0.0f};
    };

     struct RenderTargetDesc
    {
        GUID id {C_INVALID_GUID};
        Extent2D extent{.width = 0,.height = 0};
        PixelFormat format = PixelFormat::RGBA8_UNORM;
    };
    
}