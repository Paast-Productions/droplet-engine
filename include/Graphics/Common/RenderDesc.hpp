/**
 * @file RenderDesc.hpp
 * @author Sebastian Lorensson
 * @brief Describes data defined by Scene and used by rendering.
 * @date 2026-11-15
 */
#pragma once

#include <cstdint>
#include "Core/Guid.hpp"

/// namespace shared by scene (CPU) side and the render (GPU) side
namespace Droplet::Common
{
    /// @brief agnostic version of pixel formats
    enum class PixelFormat
    {
        RGBA8_SRGB,
        RGBA8_UNORM
    };

    /// Offscreen render-target description. WSI will create a Vulkan render target 
    /// and store a reference to the guid on first contact
    struct RenderTargetDesc
    {
        Guid id;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        PixelFormat format = PixelFormat::RGBA8_SRGB;
    };

    /// "An image" as seen from the scene: either a loaded texture asset or a declared render target
    struct ImageRef
    {
        /// @brief flag of the kind of image this is treated as
        enum class Kind
        {
            None,
            Texture,
            RenderTarget
        };

        Kind kind = Kind::None;
        Guid id; ///< unique ID

        /// create as Texture
        static constexpr ImageRef FromTexture(Guid id) { return {.kind = Kind::Texture, .id = id}; }
        /// create as RenderTarget
        static constexpr ImageRef FromTarget(Guid id) { return {.kind = Kind::RenderTarget, .id = id}; }
    };

    /// @brief A post-process effect applied to a camera's output after rendering but before GUI
    enum class PostEffect
    {
        Grayscale, ///< amount does nothing
        Contrast, ///< amount: contrast factor (1 = unchanged)
        Invert,
        Blur, ///< amount: radius in pixels
        AutoExposure ///< amount: brightness the average is brought to, relative to mid grey (1 = 0.18)
    };

    inline const char* ToString(PostEffect effect)
    {
        switch (effect)
        {
        case PostEffect::Grayscale: return "Grayscale";
        case PostEffect::Contrast: return "Contrast";
        case PostEffect::Invert: return "Invert";
        case PostEffect::Blur: return "Blur";
        case PostEffect::AutoExposure: return "AutoExposure";
        }
        return "UNKNOWN";
    }

    /// @brief One effect of a camera's chain, as the renderer receives it.
    struct PostEffectDesc
    {
        PostEffect effect{PostEffect::Grayscale};
        float amount{1.0f};
    };
}
