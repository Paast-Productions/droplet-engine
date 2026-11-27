/**
 * @file ViewPlan.hpp
 * @brief Render::PlanViews: which cameras draw, and which render targets each view reads
 */
#pragma once

#include <vector>

#include "Core/Guid.hpp"
#include "Graphics/Render/RenderWorld.hpp"

namespace Droplet::Graphics::Render
{
    /// @brief What a camera's view draws into and reads this frame
    struct ViewPlan
    {
        bool drawn = false;                         ///< false: no target, or one an earlier camera draws
        std::vector<GUID> readIds;         ///< other views' targets, this frame's contents
        std::vector<GUID> previousReadIds; ///< other views' targets as last frame's copy (breaks a loop)
        bool needsHistory = false;                  ///< the target keeps a previous-frame copy
    };

    /**
     * @brief Plans every camera's view of the world
     * @details A read that would close a loop between views becomes a read of last frame's copy
     * @param world The frame's cameras and the objects they see
     * @returns One plan per camera, in the world's order
     */
    [[nodiscard]] std::vector<ViewPlan> PlanViews(const RenderWorld& world);
}
