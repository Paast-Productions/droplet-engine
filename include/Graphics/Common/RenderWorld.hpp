/**
 * @file RenderWorld.hpp
 * @brief Common header for a render world extracted from the scene
 * @date 15-11-2026
 */
#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Common/RenderDesc.hpp"
#include "Core/Guid.hpp"


namespace Droplet::Graphics
{
    /// @brief GUID of the default scene shader, drawn with by objects without a (working) shader. A shader asset under
    ///        this GUID in the asset store replaces the renderer's built-in one (Shaders/scene.slang).
    inline constexpr Guid DEFAULT_SCENE_SHADER = Guid::FromName("shader/scene");

    /// @brief Half the edge of the placeholder cube the renderer draws for a mesh it does not have. Extract culls an
    ///        object whose mesh is not in the storage with this cube's bounds.
    inline constexpr float PLACEHOLDER_CUBE_HALF_EXTENT = 0.5f;

    /// A single object (mesh) in the RenderWorld
    struct RenderObject
    {
        glm::mat4 model{1.0f};
        std::uint32_t sourceNode = 0;  ///< index into Scene::nodes; a camera skips its own node's object
        Guid meshId;                   ///< MeshAsset id
        Common::ImageRef imageTexture; ///< texture asset or render target, by id
        Guid shaderId;                 ///< ShaderAsset id; invalid: the default scene shader
        std::uint64_t meshRevision{ 0 };    ///< the storage's revision of meshId; 0: not in the storage
        std::uint64_t textureRevision{ 0 }; ///< the storage's revision of a texture imageTexture
        std::uint64_t shaderRevision{ 0 };  ///< the storage's revision of shaderId
    };

    /// A single rendering camera
    struct RenderView
    {
        std::string name; ///< name, primarily for debug
        glm::mat4 view{1.0f}; ///< camera view matrix
        glm::mat4 proj{1.0f}; ///< camera projection matrix
        std::vector<std::uint32_t> visible; ///< indices into RenderWorld::objects

        Guid targetId;              ///< RenderTargetDesc id of what this camera draws to
        bool needsHistory = false;  ///< the target keeps a previous-frame copy: this view or a loop-breaking read samples it
        std::vector<Guid> readIds;  ///< other targets sampled by visible objects, this frame's contents
        /// other views' targets sampled as last frame's copy, because reading this frame's would close a loop between
        /// views (e.g. C sees A's screen while A sees C's)
        std::vector<Guid> previousReadIds;
        std::vector<Common::PostEffectDesc> effects; ///< enabled effects of the node's PostProcessComponent
    };

    /// @brief A run of text. The renderer lays it out with the font, so the world needs no font data.
    struct UiText
    {
        std::string text;            ///< '\n' starts a new line
        Guid font;                   ///< FontAsset id; one the renderer does not have draws nothing
        float pixelHeight{ 32.0f };  ///< sharp up to the size the font was baked at
        glm::vec2 alignment{ 0.0f }; ///< the point of the parent rectangle the text box is pinned to, 0..1 (0,0 top-left)
    };

    /// @brief A UI rectangle in draw order: a fill, an image, or a text run (which has no fill of its own).
    ///        The renderer turns anchors and offsets into pixels, so the world does not depend on the window size.
    struct UiRect
    {
        glm::vec2 anchorMin{ 0.0f }; ///< in the parent's rectangle, 0..1
        glm::vec2 anchorMax{ 0.0f }; ///< in the parent's rectangle, 0..1
        glm::vec2 offsetMin{ 0.0f }; ///< pixels added to anchorMin
        glm::vec2 offsetMax{ 0.0f }; ///< pixels added to anchorMax
        glm::vec4 color{ 1.0f };     ///< sRGB with alpha: the fill colour, the tint of the image, or the text colour
        std::int32_t parent{ -1 };   ///< index of the parent in RenderWorld::uiRects (always earlier); -1: the screen
        Common::ImageRef image;      ///< texture asset or render target, by id; Kind::None: a plain fill
        std::uint64_t textureRevision{ 0 }; ///< the storage's revision of a texture image
        glm::vec2 uvMin{ 0.0f };     ///< the part of the image shown: its top-left corner, 0..1
        glm::vec2 uvMax{ 1.0f };     ///< the part of the image shown: its bottom-right corner, 0..1
        /// @brief Set: a text run instead of a fill or image. Only `color` and `parent` are used (the glyphs are laid
        ///        out around `text->alignment` of the parent); no entry in the draw list but the glyphs.
        std::optional<UiText> text;
    };

    /// Snapshot of the scene at a given frame
    struct RenderWorld
    {
        std::vector<RenderObject> objects; ///< meshes
        std::vector<RenderView> views; ///< cameras
        /// Every render target the world refers to, copied from the scene
        std::vector<Common::RenderTargetDesc> targets;
        std::vector<UiRect> uiRects; ///< back to front: roots by layer, each followed by its children
        Guid presentedId;       ///< target the frame presents
        /// the storage's revision of its override of the default scene shader (Render::DEFAULT_SCENE_SHADER);
        /// 0: no override, the renderer's built-in one is used
        std::uint64_t defaultShaderRevision{ 0 };
    };
}
