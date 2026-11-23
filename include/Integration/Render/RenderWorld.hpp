#pragma once
#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Graphics/Common/RenderData.hpp"
#include "Integration/Common/RenderDesc.hpp"
#include "Integration/Core/Guid.hpp"

namespace Render
{
    /**
     * @brief GUID of the default scene shader, for objects without a working shader
     * @details A shader asset under this GUID replaces the renderer's built-in one (Shaders/scene.slang)
     */
    inline constexpr Droplet::GUID DEFAULT_SCENE_SHADER = GuidFromName("shader/scene");

    /// @brief Half the edge of the placeholder cube drawn for a mesh the GPU does not have
    inline constexpr float PLACEHOLDER_CUBE_HALF_EXTENT = 0.5f;

    /// @brief How a dynamic draw's vertices are laid out
    enum class DynamicVertexLayout : std::uint8_t
    {
        P3N3UV2, ///< position, normal, uv: drawn with a scene shader and material
        P3C4     ///< position, RGBA colour: drawn unlit and alpha blended (debug lines and shapes)
    };

    enum class DynamicTopology : std::uint8_t
    {
        Triangles,
        Lines
    };

    /// @brief Geometry rebuilt every frame (particles, debug shapes): sent as data, nothing stays on the GPU
    struct DynamicDraw
    {
        DynamicVertexLayout layout = DynamicVertexLayout::P3N3UV2;
        DynamicTopology topology = DynamicTopology::Triangles;
        std::vector<std::byte> vertexData;  ///< tightly packed vertices of `layout`
        std::vector<std::uint32_t> indices; ///< into vertexData; empty: drawn in vertex order
        glm::mat4 transform{1.0f};
        Common::ImageRef imageTexture;      ///< P3N3UV2 only: a texture asset (a render target shows the placeholder)
        Droplet::GUID shaderId{ Droplet::C_INVALID_GUID }; ///< P3N3UV2 only; invalid: the default scene shader
        bool transparent = false;           ///< P3N3UV2: alpha blended without depth writes, after the opaque draws
        bool depthTest = true;              ///< false: drawn over everything, after the rest
    };

    /// @brief A view's name and post-process effects, by the target it draws to
    struct ViewSettings
    {
        Droplet::GUID targetId{ Droplet::C_INVALID_GUID };
        std::string name;                            ///< for graph and debug names
        std::vector<Common::PostEffectDesc> effects; ///< applied to the view's output, in order
    };

    /// @brief Everything one frame draws
    struct RenderWorld
    {
        std::vector<Droplet::Graphics::CameraRenderData> cameras; ///< each with the objects it sees, culled
        std::vector<DynamicDraw> dynamicDraws;  ///< shown by the cameras with `debug` set
        std::vector<ViewSettings> viewSettings; ///< optional; a view without one is named after its target
        Droplet::GUID presentedId{ Droplet::C_INVALID_GUID }; ///< the target the window shows outside the editor
        float time = 0.0f; ///< seconds, for shaders
    };

    // TODO:: convert RenderData's shader slots to one shader module, until pipelines are built per stage combination
    /**
     * @brief The shader module an object draws with
     * @param object An object a camera sees
     * @returns The module in both the vertex and fragment slot; invalid (the default scene shader) otherwise
     */
    constexpr Droplet::GUID ShaderOf(const Droplet::Graphics::RenderData& object) noexcept
    {
        return object.vsData.GUID == object.fsData.GUID ? object.vsData.GUID : Droplet::C_INVALID_GUID;
    }

    // TODO: convert RenderData's texture to the renderer's image reference
    /**
     * @brief The image an object shows
     * @param texture An object's texture data
     * @returns A render target, a texture or no image
     */
    constexpr Common::ImageRef ImageOf(const Droplet::Graphics::TextureData& texture) noexcept
    {
        if (texture.GUID == Droplet::C_INVALID_GUID)
            return {};
        return texture.isRenderTarget ? Common::ImageRef::FromTarget(texture.GUID) : Common::ImageRef::FromTexture(texture.GUID);
    }
}
