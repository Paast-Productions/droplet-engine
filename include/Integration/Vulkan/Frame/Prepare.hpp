/// @file Prepare.hpp
/// @brief Vulkan::Prepare: resolves the RenderWorld's GUIDs to GPU handles and pipelines.
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Vulkan/Passes/DrawPass.hpp"
#include "Integration/Vulkan/Resources/GpuResourceManager.hpp"
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"
#include "Integration/Render/PreparedFrame.hpp"
#include "Integration/Render/RenderWorld.hpp"

namespace Vulkan
{
    /// @brief Helper struct of resources, what `Prepare` reads and fills    
    struct PrepareContext
    {
        GpuResourceManager& gpu;
        ShaderLibrary& shaders;
        PipelineLibrary& pipelines;
        const DrawPass& scenePass; ///< scene material interface (MakePipelineDesc)
        vk::Format depthFormat;    ///< of the views' depth transients
    };

    /**
     * @brief Builds one draw list per view of the world, GUIDs get converted to GPU handles, and fallbacks are chosen.
     * @details Views are chosen in the world's order. Fallbacks are selected if something doesn't work out so that something exists on the GPU for the draw call.
     * Fallbacks:
     * - a mesh that is not on the GPU (yet): the placeholder cube;
     * - a texture that is not on the GPU (yet), or a render target the scene does not declare: the placeholder (checker) texture; no texture at all: white;
     * - a shader that has not arrived or does not compile: the default scene shader;
     * - a render target the view itself draws to, or one that would close a loop between views: its history (last frame's copy);
     * - an object left without a pipeline (not even the default shader compiled): not in the list;
     * - a view whose target does not exist: no draw list
     */
    [[nodiscard]] Render::PreparedFrame Prepare(const Render::RenderWorld& world, const PrepareContext& context);
}
