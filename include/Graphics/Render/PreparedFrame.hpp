/// @file PreparedFrame.hpp
/// @brief What Vulkan::Prepare produces: a draw list per view (sorted to minimise rebinding) and the UI's, by GPU handle.
/// Render layer, no Vulkan; Vulkan::DrawPass and Vulkan::UiPass consume it.
#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "Common/RenderDesc.hpp"
#include "Render/Handles.hpp"

namespace Render
{
    /// @brief The image something is drawn with: a texture or a render target, never both.
    /// @details Read it with std::get_if<TextureHandle> / std::get_if<TargetHandle>. In a draw list the handle is
    ///          valid. A default one holds an invalid texture handle: the passes draw the placeholder texture for it.
    using ImageBinding = std::variant<TextureHandle, TargetHandle>;

    // == Views ==

    /// One draw of a view. Every handle is valid: an object that cannot be drawn is not in the list
    struct DrawItem
    {
        glm::mat4 model{1.0f};   ///< object to world
        PipelineHandle pipeline; ///< the object's shader (or the default one), for the view's attachment formats
        MeshHandle mesh;         ///< the object's mesh, or the placeholder cube
        ImageBinding material;   ///< a texture (the object's, placeholder or white), or a render target or history
    };

    /// @brief Everything needed to render one view: where it draws, what it samples and its draws.
    /// @details Handles are valid unless a field says otherwise; a view whose target does not exist has no draw list.
    struct DrawList
    {
        std::string name;        ///< the view's name, for graph and debug names
        glm::mat4 viewProj{1.0f}; ///< projection * view of the camera

        TargetHandle target;     ///< what the view draws to
        TargetHandle history;    ///< previous-frame copy of target, refreshed after the view; invalid: none is kept
        bool presented = false;  ///< the frame presents this view's target (at most one list of a frame)

        std::vector<TargetHandle> reads;         ///< other views' targets its draws sample, this frame's contents
        std::vector<TargetHandle> previousReads; ///< other views' histories its draws sample (loop-breaking reads)
        std::vector<Common::PostEffectDesc> effects; ///< applied to the view's output, in order

        /// ordered by pipeline, then material (textures, render targets), then mesh, so consecutive draws share
        /// pipelines, descriptor sets and vertex buffers
        std::vector<DrawItem> items;
    };

    // == UI ==

    /// @brief One UI rectangle to draw: a Render::UiRect (Render/RenderWorld.hpp) with its image resolved. The
    ///        renderer turns the anchors and offsets into pixels for the screen size it draws at.
    struct UiDrawItem
    {
        glm::vec2 anchorMin{ 0.0f }; ///< in the parent's rectangle, 0..1
        glm::vec2 anchorMax{ 0.0f }; ///< in the parent's rectangle, 0..1
        glm::vec2 offsetMin{ 0.0f }; ///< pixels added to anchorMin
        glm::vec2 offsetMax{ 0.0f }; ///< pixels added to anchorMax
        glm::vec4 color{ 1.0f };     ///< sRGB with alpha: the fill colour, or the tint of the image
        std::int32_t parent{ -1 };   ///< index of the parent in UiDrawList::items (always earlier); -1: the screen
        glm::vec2 uvMin{ 0.0f };     ///< the part of the image shown: its top-left corner, 0..1
        glm::vec2 uvMax{ 1.0f };     ///< the part of the image shown: its bottom-right corner, 0..1
        ImageBinding image;          ///< what it shows; a plain fill: the white texture
    };

    /// @brief The frame's UI, drawn on top of the presented image.
    struct UiDrawList
    {
        std::vector<UiDrawItem> items;   ///< back to front: one per fill or image of RenderWorld::uiRects, one per glyph of its text; `parent` indexes this list
        std::vector<TargetHandle> reads; ///< render targets the items' images show, each once
    };

    // == Frame ==

    /// @brief Everything the renderer draws in a frame; what Vulkan::Prepare returns.
    struct PreparedFrame
    {
        std::vector<DrawList> views; ///< in the world's view order (the frame graph orders the passes)
        UiDrawList ui;
    };
}
