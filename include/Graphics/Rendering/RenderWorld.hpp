/**
 * @file RenderWorld.hpp
 * @brief RenderWorld is a snapshot of a scene at a given time/frame.
 */

#pragma once

#include "Core/Guid.hpp"
#include "Graphics/Common/RenderDesc.hpp"

namespace Droplet::Render
{
    
    /// Represents a single renderable object in the scene
    struct RenderObject
    {
        // world transform
        glm::mat4 transform{1.0f};
        
        // mesh
        Guid meshId;
        
        // texture
        Common::ImageRef imageTexture;
        
        // shader(s)
        Guid shaderId;
    };
    
    struct RenderView
    {
            
    };
}
