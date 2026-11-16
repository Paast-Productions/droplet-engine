/** 
 * @file Handles.hpp
 * @brief Handles to GPU-side objects as the render side sees them.
 * @details The scene should never seee these, it refers to assets and targets by Guid. API-agnostic, no Vulkan here.
 */
#pragma once

#include "Core/Handle.hpp"

namespace Droplet::Render
{
    struct MeshTag;
    struct TextureTag;
    struct TargetTag;
    struct ShaderTag;
    struct PipelineTag;

    using MeshHandle = Handle<MeshTag>;
    using TextureHandle = Handle<TextureTag>;
    using TargetHandle = Handle<TargetTag>;
    using ShaderHandle = Handle<ShaderTag>;
    using PipelineHandle = Handle<PipelineTag>;
}
