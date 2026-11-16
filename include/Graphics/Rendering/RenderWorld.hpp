/**
 * @file RenderWorld.hpp
 * @brief RenderWorld is a snapshot of a scene at a given time/frame.
 */

#pragma once

#include "Core/Guid.hpp"
#include "Common/RenderDesc.hpp"

namespace Droplet::Render
{
    /// Represents a single renderable object in the scene
    struct RenderObject
    {
        glm::mat4 model{1.0f}; ///< model matrix
        Guid meshId; ///< MeshAsset id
        Common::ImageRef imageTexture; ///< texture asset or render target, by id
        Guid shaderId; ///< ShaderAsset id
    };

    /// Represents a camera in the scene and what it sees
    struct RenderView
    {
        std::string name; ///< name, primarily for debug
        
        glm::mat4 view{1.0f}; ///< camera view matrix
        glm::mat4 proj{1.0f}; ///< camera projection matrix
        std::vector<std::uint32_t> visible; ///< indices into RenderWorld::objects

        Guid targetId; ///< RenderTargetDesc id of what this camera draws to
        bool needsHistory = false; ///< true if depends on history
        
        ///< the target keeps a previous-frame copy: this view or a loop-breaking read samples it
        std::vector<Guid> readIds; ///< other targets sampled by visible objects, this frame's contents
        /// other views' targets sampled as last frame's copy, because reading this frame's would close a loop between
        /// views (e.g. C sees A's screen while A sees C's)
        std::vector<Guid> previousReadIds;
        std::vector<Common::PostEffectDesc> effects; ///< enabled effects of the node's PostProcessComponent
    };

    /// a rectangle with a position/offset `x`, y and a width height from x y
    struct Rect
    {
        glm::vec2 x{0.f};
        glm::vec2 y{0.f};
        glm::vec2 width{0.f};
        glm::vec2 height{0.f};
    };

    /// @brief A UI rectangle in draw order
    /// @details a fill, an image, OR one glyph (an image cut out of the font atlas). The renderer turns anchors and 
    /// offsets into pixels, so that thee world does not depend on the window size.
    struct UiRect
    {
        Rect rect{
            .x = glm::vec2{1.f},
            .y = glm::vec2{1.f},
            .width = glm::vec2{1.f},
            .height = glm::vec2{1.f}
        }; ///< x and y is parent rectangle [0, 1], width and height are pixels added to anchor
        
        std::int32_t parent{-1}; ///< index of the parent in RenderWorld::uiRects (always earlier); -1: the screen
        
        glm::vec4 color{1.0f}; ///< sRGB with alpha: the fill colour, or the tint of the image
        Common::ImageRef image; ///< texture asset or render target, by id; Kind::None: a plain fill
        glm::vec2 uvMin{0.0f}; ///< the part of the image shown: its top-left corner, 0..1
        glm::vec2 uvMax{1.0f}; ///< the part of the image shown: its bottom-right corner, 0..1
    };

    /// Snapshot of the scene at a given frame
    struct RenderWorld
    {
        std::vector<RenderObject> objects; ///< meshes
        std::vector<RenderView> views; ///< cameras
        
        /// Every render target the world refers to, copied from the scene
        std::vector<Common::RenderTargetDesc> targets;
        std::vector<UiRect> uiRects; ///< back to front: roots by layer, each followed by its children
        Guid presentedId; ///< target the frame presents
    };
}
