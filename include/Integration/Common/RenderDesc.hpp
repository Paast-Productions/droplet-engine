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

#include "Integration/Core/Guid.hpp"

/// namespace shared by scene (CPU) side and the render (GPU) side
namespace Common
{
    // == Images and render targets ==

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
        Droplet::GUID id{ Droplet::C_INVALID_GUID };
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
        Droplet::GUID id{ Droplet::C_INVALID_GUID };

        /// create as Texture
        static constexpr ImageRef FromTexture(Droplet::GUID id) { return { Kind::Texture, id }; }
        /// create as RenderTarget
        static constexpr ImageRef FromTarget(Droplet::GUID id) { return { Kind::RenderTarget, id }; }
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
        bool repeat = true;                           ///< false: clamp to the edge

        friend bool operator==(const TextureSampling&, const TextureSampling&) = default;
    };

    /// A scene shader: a Slang module with a vertex and a fragment entry point. It must follow the scene material
    /// interface (Vulkan::DrawPass): vertex layout Common::MeshVertex, set 0 binding 0 base colour, ScenePushConstants.
    struct ShaderDesc
    {
        std::string moduleName;               ///< Slang module name
        std::filesystem::path sourcePath;     ///< relative to the working directory
        std::string vertexEntry = "vMain";
        std::string fragmentEntry = "fMain";
    };
}
