/// @file Prepare.hpp
/// @brief Vulkan::Prepare: resolves the RenderWorld's GUIDs to GPU handles and pipelines.
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Vulkan/Passes/DrawPass.hpp"
#include "Vulkan/Resources/FontLibrary.hpp"
#include "Vulkan/Resources/GpuResourceManager.hpp"
#include "Vulkan/Resources/PipelineLibrary.hpp"
#include "Render/PreparedFrame.hpp"
#include "Render/RenderWorld.hpp"

namespace Vulkan
{
    /// @brief What Prepare reads and fills.
    struct PrepareContext
    {
        GpuResourceManager& gpu;
        ShaderLibrary& shaders;
        const FontLibrary& fonts;  ///< text is laid out here
        PipelineLibrary& pipelines;
        const DrawPass& scenePass; ///< scene material interface (MakePipelineDesc)
        vk::Format depthFormat;    ///< of the views' depth transients
    };

    /// @brief Builds one draw list per view of `world`, in the world's order, and the UI's. The only place GUIDs become
    ///        GPU handles, and the only place fallbacks are chosen:
    ///        - a mesh that is not on the GPU (yet): the placeholder cube;
    ///        - a texture that is not on the GPU (yet), or a render target the scene does not declare: the
    ///          placeholder (checker) texture; no texture at all: white (objects and UI rectangles alike);
    ///        - a shader that has not arrived or does not compile: the default scene shader;
    ///        - a render target the view itself draws to, or one that would close a loop between views: its history
    ///          (last frame's copy);
    ///        - an object left without a pipeline (not even the default shader compiled): not in the list;
    ///        - a view whose target does not exist: no draw list;
    ///        - text whose font or font atlas is not on the GPU: no glyphs.
    ///        So every handle in the result refers to something that exists on the GPU.
    /// @details Never reads an asset storage; uploads arrive separately (Renderer::Upload).
    [[nodiscard]] Render::PreparedFrame Prepare(const Render::RenderWorld& world, const PrepareContext& context);
}
