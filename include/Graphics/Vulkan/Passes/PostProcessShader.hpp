/// @file PostProcessShader.hpp
/// @brief Vulkan::PostProcess: the C++ side of the post-process shaders (Shaders/Modules/post_process.md.slang): the
///        effect codes and the push constants they receive.
#pragma once
#include <cstdint>

namespace Droplet::Graphics::Vulkan::PostProcess
{
    /// @brief Effect a shader pass applies; must match PostProcess::Effect in Shaders/Modules/post_process.md.slang
    ///        (the Slang enum is numbered by declaration order).
    enum class Effect : std::uint32_t
    {
        Grayscale = 0,
        Contrast = 1,
        Invert = 2,
        Passthrough = 3,
        BlurHorizontal = 4,   ///< compute shader only
        BlurVertical = 5,     ///< compute shader only
        MeasureLuminance = 6, ///< compute shader only: writes the average luminance into the step buffer
        ApplyExposure = 7     ///< compute shader only: reads the step buffer
    };

    constexpr const char* ToString(Effect effect)
    {
        switch (effect)
        {
        case Effect::Grayscale: return "Grayscale";
        case Effect::Contrast: return "Contrast";
        case Effect::Invert: return "Invert";
        case Effect::Passthrough: return "Passthrough";
        case Effect::BlurHorizontal: return "BlurHorizontal";
        case Effect::BlurVertical: return "BlurVertical";
        case Effect::MeasureLuminance: return "MeasureLuminance";
        case Effect::ApplyExposure: return "ApplyExposure";
        }
        return "Unknown";
    }

    /// @brief Push constants of the post-process fragment and compute shaders.
    struct PushConstants
    {
        Effect effect = Effect::Passthrough;
        float amount = 1.0f;
    };
    static_assert(sizeof(PushConstants) == 8);
}
