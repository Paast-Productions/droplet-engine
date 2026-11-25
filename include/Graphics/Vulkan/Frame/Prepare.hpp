/// @file Prepare.hpp
/// @brief Vulkan::Prepare: resolves the RenderWorld's GUIDs to GPU handles and pipelines.
#pragma once

#include <cstdint>
#include <span>
#include <unordered_set>

#include <vulkan/vulkan_raii.hpp>

#include "Graphics/Vulkan/Passes/DrawPass.hpp"
#include "Graphics/Vulkan/Resources/GpuResourceManager.hpp"
#include "Graphics/Vulkan/Resources/PipelineLibrary.hpp"
#include "Graphics/Render/PreparedFrame.hpp"
#include "Graphics/Render/RenderWorld.hpp"
#include "Graphics/Render/ViewPlan.hpp"

namespace Droplet::Graphics::Vulkan
{
    /// @brief Helper struct of resources, what `Prepare` reads and fills    
    struct PrepareContext
    {
        GpuResourceManager& gpu;
        ShaderLibrary& shaders;
        PipelineLibrary& pipelines;
        const DrawPass& scenePass; ///< scene material interface (MakePipelineDesc)
        vk::Format depthFormat;    ///< of the views' depth transients
        std::unordered_set<std::uint64_t>& reportedMeshes; ///< dynamic meshes and shaders already reported as not drawable
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
