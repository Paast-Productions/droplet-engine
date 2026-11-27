#pragma once
#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Common/RenderData.hpp"
#include "Common/RenderDesc.hpp"
#include "Core/Guid.hpp"

namespace Droplet::Graphics::Render
{
    /// @brief Half the edge of the placeholder cube drawn for a mesh the GPU does not have
    inline constexpr float PLACEHOLDER_CUBE_HALF_EXTENT = 0.5f;

    /// @brief A view's name and post-process effects, by the target it draws to
    struct ViewSettings
    {
        GUID targetId{ C_INVALID_GUID };
        std::string name;                            ///< for graph and debug names
        std::vector<Common::PostEffectDesc> effects; ///< applied to the view's output, in order
    };

    /// @brief Everything one frame draws
    struct RenderWorld
    {
        std::vector<Common::CameraRenderData> cameras; ///< each with the objects it sees, culled
        std::vector<ViewSettings> viewSettings; ///< optional; a view without one is named after its target
        GUID presentedId{ C_INVALID_GUID }; ///< the target the window shows outside the editor
        GUID defaultVertexShader{ C_INVALID_GUID };   ///< for objects without working shaders
        GUID defaultFragmentShader{ C_INVALID_GUID }; ///< for objects without working shaders
        float time = 0.0f; ///< seconds, for shaders
    };
}
