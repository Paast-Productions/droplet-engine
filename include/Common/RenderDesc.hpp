/**
 * @file RenderDesc.hpp
 * @author Sebastian Lorensson
 * @brief Describes data defined by Scene and used by rendering.
 * @date 2026-11-15
 */
#pragma once
#include <cstdint>
#include <filesystem>
#include <string>

#include <glm/glm.hpp>

#include "Core/Guid.hpp"

/// namespace shared by scene (CPU) side and the render (GPU) side
namespace Droplet::Common
{
    // == Images and render targets ==

    /// @brief Production's pixel formats
    enum class PixelFormat
    {
        RGBA8_SRGB,
        RGBA8_UNORM,
        RGBA32_FLOAT
    };

    // == Post-processing ==

    /// @brief A post-process effect applied to a camera's output after rendering but before GUI
    enum class PostEffect
    {
        Grayscale,   ///< amount does nothing
        Contrast,    ///< amount: contrast factor (1 = unchanged)
        Invert,
        Blur,        ///< amount: radius in pixels
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

    // == Asset data, as the asset storage keeps it and the renderer receives it ==

    /// Describes a single vertex of a mesh. Uploaded as-is, so it matches the GPU's vertex layout
    /// (Vulkan::Vertex::P3N3UV2)
    struct MeshVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    /// @brief How to blend texture's texels when sampling
    enum class TextureFilter
    {
        Linear,  ///< smooth; mip levels are blended too
        Nearest  ///< blocky (pixel art); the nearest mip level is used
    };

    /// @brief How a texture is sampled; chosen per texture, like its pixels
    struct TextureSampling
    {
        bool generateMips = true;                     ///< a full mip chain is built when the texture is uploaded
        TextureFilter filter = TextureFilter::Linear; ///< @copydoc Common::TextureFilter
        float maxAnisotropy = 1.0f;                   ///< 1: off; clamped to what the device supports
        bool repeat = false;                           ///< false: clamp to the edge

        friend bool operator==(const TextureSampling&, const TextureSampling&) = default;
    };
    
    struct Extent2D{
        std::uint32_t width{0};
        std::uint32_t height{0};
    };
    
    struct RenderTargetDesc
    {
        GUID id {C_INVALID_GUID};
        Extent2D extent{.width = 0,.height = 0};
        PixelFormat format = PixelFormat::RGBA8_UNORM;
    };

    /// A scene shader: a Slang module with a vertex and a fragment entry point. It must follow the scene material
    /// interface (Vulkan::DrawPass): vertex layout Common::MeshVertex, Graphics::PushConstantData, set 0 the view, set 2 the textures.
    struct ShaderDesc
    {
        std::string moduleName;               ///< Slang module name
        std::filesystem::path sourcePath;     ///< relative to the working directory
        std::string vertexEntry = "vMain";
        std::string tessCtrlEntry = "tcMain";
        std::string tessEvalEntry = "teMain";
		std::string geometryEntry = "gMain";
        std::string fragmentEntry = "fMain";
    };
}
