/// @file
/// @brief Handles to GPU-side objects as the render side sees them. API-agnostic, no Vulkan here
/// The scene never sees these, it refers to assets and targets by Guid
#pragma once

#include "Integration/Core/Handle.hpp"

namespace Render
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