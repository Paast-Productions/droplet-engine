#pragma once
#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Integration/Common/RenderDesc.hpp"
#include "Integration/Core/Guid.hpp"

namespace Render
{
    /// @brief GUID of the default scene shader, drawn with by objects without a (working) shader. A shader asset under
    ///        this GUID in the asset store replaces the renderer's built-in one (Shaders/scene.slang).
    inline constexpr Droplet::GUID DEFAULT_SCENE_SHADER = GuidFromName("shader/scene");

    /// @brief Half the edge of the placeholder cube the renderer draws for a mesh it does not have. Extract culls an
    ///        object whose mesh is not in the storage with this cube's bounds.
    inline constexpr float PLACEHOLDER_CUBE_HALF_EXTENT = 0.5f;

    /// A single object (mesh) in the RenderWorld
    struct RenderObject
    {
        glm::mat4 transform{1.0f};
        std::uint32_t sourceNode = 0;  ///< index into Scene::nodes; a camera skips its own node's object
        Droplet::GUID meshId{ Droplet::C_INVALID_GUID }; ///< MeshAsset id
        Common::ImageRef imageTexture; ///< texture asset or render target, by id
        Droplet::GUID shaderId{ Droplet::C_INVALID_GUID }; ///< ShaderAsset id; invalid: the default scene shader
        std::uint64_t meshRevision{ 0 };    ///< the storage's revision of meshId; 0: not in the storage
        std::uint64_t textureRevision{ 0 }; ///< the storage's revision of a texture imageTexture
        std::uint64_t shaderRevision{ 0 };  ///< the storage's revision of shaderId
        bool transparent = false;           ///< alpha blended, drawn after the opaque objects without depth writes
    };

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

    /// A single rendering camera
    struct RenderView
    {
        std::string name; ///< name, primarily for debug
        glm::mat4 view{1.0f}; ///< camera view matrix
        glm::mat4 proj{1.0f}; ///< camera projection matrix
        glm::vec4 clearColor{0.05f, 0.05f, 0.08f, 1.0f}; ///< the target's colour before the view draws
        std::vector<std::uint32_t> visible; ///< indices into RenderWorld::objects

        Droplet::GUID targetId{ Droplet::C_INVALID_GUID }; ///< RenderTargetDesc id of what this camera draws to
        bool needsHistory = false;  ///< the target keeps a previous-frame copy: this view or a loop-breaking read samples it
        std::vector<Droplet::GUID> readIds; ///< other targets sampled by visible objects, this frame's contents
        /// other views' targets sampled as last frame's copy, because reading this frame's would close a loop between
        /// views (e.g. C sees A's screen while A sees C's)
        std::vector<Droplet::GUID> previousReadIds;
        std::vector<Common::PostEffectDesc> effects; ///< enabled effects of the node's PostProcessComponent
        std::vector<std::uint32_t> dynamicVisible;   ///< indices into RenderWorld::dynamicDraws
    };

    /// Snapshot of the scene at a given frame
    struct RenderWorld
    {
        std::vector<RenderObject> objects; ///< meshes
        std::vector<RenderView> views; ///< cameras
        std::vector<DynamicDraw> dynamicDraws; ///< this frame's dynamic geometry, shown by the views that list it
        /// Every render target the world refers to, copied from the scene
        std::vector<Common::RenderTargetDesc> targets;
        Droplet::GUID presentedId{ Droplet::C_INVALID_GUID }; ///< target the frame presents
        /// the storage's revision of its override of the default scene shader (Render::DEFAULT_SCENE_SHADER);
        /// 0: no override, the renderer's built-in one is used
        std::uint64_t defaultShaderRevision{ 0 };
        float time = 0.0f; ///< seconds, for shaders
    };
}
