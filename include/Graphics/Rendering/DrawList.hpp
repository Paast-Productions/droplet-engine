/**
 * @file DrawList.hpp
 * @brief What to draw for one view, sorted to minimise rebinding, and the per-draw data scene shaders receive.
 * No vulkan, consumed by DrawPass
 */
#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

#include "Common/RenderDesc.hpp"
#include "Common/Handles.hpp"


namespace Droplet::Graphics
{
    /// @brief Per-draw data scene shaders receive.
    struct ScenePushConstants
    {
        glm::mat4 viewProj;
        glm::mat4 model;
    };
    static_assert(sizeof(ScenePushConstants) == 128, "Push constants must be 128 bytes.");

    
    ScenePushConstants MakePushConstants(const RenderView &view, const RenderObject &object);
    
    
    
}