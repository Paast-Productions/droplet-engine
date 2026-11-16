/**
* @file RenderDesc.hpp
* @brief Shared vocabulary between scene, resource manager and rendering.
*/
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include <glm/glm.hpp>

#include "Core/Guid.hpp"

/**
 * @namespace Droplet::Common
 * @brief vocabulary between scene / cpu resources and rendering / gpu resources
 */
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

        Kind kind = Kind::None; ///< kind of texture
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

    /// Helper function to get PostEffect enum value as a string
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

    /// @brief One effect of a camera's chain, as the renderer receives it, what gets passed as push constants
    struct PostEffectDesc
    {
        PostEffect effect{PostEffect::Grayscale}; ///< slang can handle enums. PostEffect enum should match slang's 
        float amount{1.0f}; ///< can be treated as generic value
    };
    
    // == resource data as the resource manager stores it and what gets uploaded to the rendering ==
    
    /// @brief single vertex of a mesh
    struct MeshVertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };
    
    /// @brief How to blend texture's texels when sampling
    enum class TextureFilter
    {
        Linear,  ///< smooth, mip levels are blended too
        Nearest  ///< blocky, the nearest mip level is used
    };

    /// @brief How a texture is sampled; chosen per texture, like its pixels
    struct TextureSampling
    {
        bool generateMips = true;                     ///< a full mip chain is built when the texture is uploaded
        TextureFilter filter = TextureFilter::Linear; ///< @copydoc Common::TextureFilter
        float maxAnisotropy = 1.0f;                   ///< defaults to off. Clamped to what the device supports
        bool repeat = true;                           ///< false, clamp to the edge

        friend bool operator==(const TextureSampling&, const TextureSampling&) = default;
    };
    
    /// A scene shader: a Slang module with a vertex and a fragment entry point. It
    /// Describes a single slang shader with a vertex and fragment shader entry.
    /// must follow the scene material interface (Vulkan::DrawPass): 
    /// vertex layout Common::MeshVertex, set 0 binding 0 base colour, ScenePushConstants.
    struct ShaderDesc
    {
        std::string moduleName;               ///< Slang module name
        std::filesystem::path sourcePath;     ///< relative to the working directory
        std::string vertexEntry = "vMain";    ///< entry point in shader  
        std::string fragmentEntry = "fMain";  ///< entry point in shader
    };
}
