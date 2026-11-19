#pragma once
#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "Integration/Common/RenderDesc.hpp"
#include "Integration/Core/Guid.hpp"

namespace Render
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
        glm::mat4 transform{1.0f};
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

    /// Snapshot of the scene at a given frame
    struct RenderWorld
    {
        std::vector<RenderObject> objects; ///< meshes
        std::vector<RenderView> views; ///< cameras
        /// Every render target the world refers to, copied from the scene
        std::vector<Common::RenderTargetDesc> targets;
        Guid presentedId;       ///< target the frame presents
        /// the storage's revision of its override of the default scene shader (Render::DEFAULT_SCENE_SHADER);
        /// 0: no override, the renderer's built-in one is used
        std::uint64_t defaultShaderRevision{ 0 };
    };
}
