/// @file Prepare.hpp
/// @brief Vulkan::Prepare: resolves the RenderWorld's GUIDs to GPU handles and pipelines.
#pragma once

#include <span>

#include <vulkan/vulkan_raii.hpp>

#include "Integration/Vulkan/Passes/DrawPass.hpp"
#include "Integration/Vulkan/Resources/GpuResourceManager.hpp"
#include "Integration/Vulkan/Resources/PipelineLibrary.hpp"
#include "Integration/Render/PreparedFrame.hpp"
#include "Integration/Render/RenderWorld.hpp"
#include "Integration/Render/ViewPlan.hpp"

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
     * @brief Builds a draw list per drawn camera, with GPU handles and fallbacks for what is missing
     * @details Fallbacks: the placeholder cube for a mesh, the checker texture for a texture or a never rendered target,
     *          the default shader for a shader, last frame's copy for a target the view draws into or reads across a loop
     * @param world The frame's cameras and dynamic draws
     * @param plans One per camera (Render::PlanViews)
     * @param context The GPU resources, shaders and pipelines to resolve with
     * @returns The draw lists, in the world's camera order
     */
    [[nodiscard]] Render::PreparedFrame Prepare(
        const Render::RenderWorld& world, std::span<const Render::ViewPlan> plans, const PrepareContext& context);
}
