/**
 * @file RenderWorld.hpp
 * @brief Common header for a render world extracted from the scene
 * @date 15-11-2026
 */
#pragma once

namespace Droplet::Render
{
    // exactly one of the two handles is valid at any time
    struct ImageBinding
    {
        TextureHandle texture; ///< texture
        TargetHandle target; ///< RenderTarget
        bool IsTarget() const {return target.IsValid();}
    };
    
    struct RenderWorld
    {
        glm::mat4 transform{1.0f};
        GUID meshId; ///< mesh id as found in the resource manager
        
    };
}