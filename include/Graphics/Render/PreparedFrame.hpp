/// @file PreparedFrame.hpp
/// @brief What Vulkan::Prepare produces: a draw list per view (sorted to minimise rebinding), by GPU handle.
/// Render layer, no Vulkan; Vulkan::DrawPass consumes it.
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "Common/RenderDesc.hpp"
#include "Graphics/Render/Handles.hpp"

namespace Droplet::Graphics::Render
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

    /// @brief A run of PreparedFrame's packed dynamic geometry, drawn with one call
    struct DynamicDrawItem
    {
        glm::mat4 model{1.0f};
        PipelineHandle pipeline;
        ImageBinding material;
        std::uint32_t vertexByteOffset = 0; ///< where the run's vertex layout starts in dynamicVertexData
        bool indexed = false;               ///< `first` and `count` are indices (into dynamicIndices), else vertices
        std::uint32_t first = 0;
        std::uint32_t count = 0;
    };
    
    /// @brief Data needed to draw one piece of text
    struct TextDrawItem
    {
        glm::mat4 model{1.0f};
        glm::vec4 color{1.0f};
        
        TextureHandle fontTexture;
        
        uint32_t firstInstance = 0;
        uint32_t instanceCount = 0;
    };

    /// @brief Everything needed to render one view: where it draws, what it samples and its draws.
    /// @details Handles are valid unless a field says otherwise; a view whose target does not exist has no draw list.
    struct DrawList
    {
        std::string name;        ///< the view's name, for graph and debug names
        glm::mat4 view{1.0f};    ///< the camera's view matrix
        glm::mat4 proj{1.0f};    ///< the camera's projection matrix
        glm::vec4 clearColor{0.05f, 0.05f, 0.08f, 1.0f}; ///< the target's colour before the draws
        glm::vec3 cameraPosition{0.0f};                  ///< world space
        glm::vec3 cameraForward{0.0f, 0.0f, -1.0f};      ///< world space, unit length

        TargetHandle target;     ///< what the view draws to
        TargetHandle history;    ///< previous-frame copy of target, refreshed after the view; invalid: none is kept
        bool presented = false;  ///< the frame presents this view's target (at most one list of a frame)

        std::vector<TargetHandle> reads;         ///< other views' targets its draws sample, this frame's contents
        std::vector<TargetHandle> previousReads; ///< other views' histories its draws sample (loop-breaking reads)
        std::vector<Common::PostEffectDesc> effects; ///< applied to the view's output, in order

        /// opaque draws ordered by pipeline, then material (textures, render targets), then mesh, so consecutive draws
        /// share pipelines, descriptor sets and vertex buffers; then transparent draws, back to front
        std::vector<DrawItem> items;
        /// after `items`: opaque, then blended, then without depth test; neighbours that share all state are merged
        std::vector<DynamicDrawItem> dynamicItems;
        /// last: kept dynamic meshes drawn without depth test
        std::vector<DrawItem> lateItems;
        
        std::vector<TextDrawItem> textItems;
    };

    // == Frame ==

    /// @brief Everything the renderer draws in a frame; what Vulkan::Prepare returns.
    struct PreparedFrame
    {
        std::vector<DrawList> views; ///< in the world's view order (the frame graph orders the passes)
        float time = 0.0f;           ///< RenderWorld::time
        /// the streamed dynamic meshes the views show, packed once: vertices grouped by stride, indices relative to the
        /// start of their group's vertices
        std::vector<std::byte> dynamicVertexData;
        std::vector<std::uint32_t> dynamicIndices;
        
        std::vector<Text::GlyphInstance> glyphInstances;
    };
}
