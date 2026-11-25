/**
 * @file ViewPlan.cpp
 * @brief Render::PlanViews implementation: drawn cameras, render-target reads and loop breaking
 */
#include "Graphics/Render/ViewPlan.hpp"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <utility>

using namespace Droplet;

namespace
{
    /// @brief Depth-first search over "view reads the target of view", collecting the reads that close a loop
    void FindLoopReads(
        const std::vector<Graphics::Render::ViewPlan>& plans,
        const std::unordered_map<GUID, std::uint32_t>& viewOfTarget,
        std::uint32_t viewIndex,
        std::vector<std::uint8_t>& visitState, // 0 not visited, 1 on the search path, 2 done
        std::vector<std::pair<std::uint32_t, GUID>>& loopReads)
    {
        visitState[viewIndex] = 1;
        for (const GUID readId : plans[viewIndex].readIds)
        {
            const std::uint32_t ownerIndex = viewOfTarget.at(readId);
            if (visitState[ownerIndex] == 1)
                loopReads.emplace_back(viewIndex, readId);
            else if (visitState[ownerIndex] == 0)
                FindLoopReads(plans, viewOfTarget, ownerIndex, visitState, loopReads);
        }
        visitState[viewIndex] = 2;
    }
}

namespace Droplet::Graphics::Render
{
    
std::vector<ViewPlan> PlanViews(const RenderWorld& world)
{
    std::vector<ViewPlan> plans(world.cameras.size());

    // one writer per target: the first camera that draws into it
    std::unordered_map<GUID, std::uint32_t> viewOfTarget;
    for (std::uint32_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
    {
        const Common::RenderTargetDesc& target = world.cameras[cameraIndex].target;
        if (target.id == Droplet::C_INVALID_GUID || target.extent.width == 0 || target.extent.height == 0)
            continue;
        plans[cameraIndex].drawn = viewOfTarget.emplace(target.id, cameraIndex).second;
    }

    // targets the views' objects show; one no camera draws this frame shows its last image (Prepare)
    for (std::uint32_t cameraIndex = 0; cameraIndex < world.cameras.size(); ++cameraIndex)
    {
        ViewPlan& plan = plans[cameraIndex];
        if (!plan.drawn)
            continue;
        const GUID ownTarget = world.cameras[cameraIndex].target.id;
        for (const Common::RenderData& object : world.cameras[cameraIndex].renderData)
        {
            const GUID target = object.texData.GUID;
            if (!object.texData.isRenderTarget || !viewOfTarget.contains(target))
                continue;
            if (target == ownTarget)
                plan.needsHistory = true; // sees its own output
            else if (std::ranges::find(plan.readIds, target) == plan.readIds.end())
                plan.readIds.push_back(target);
        }
    }

    // removing every read that closes a loop in a depth-first search leaves no loop
    std::vector<std::uint8_t> visitState(plans.size(), 0);
    std::vector<std::pair<std::uint32_t, GUID>> loopReads;
    for (std::uint32_t cameraIndex = 0; cameraIndex < plans.size(); ++cameraIndex)
    {
        if (plans[cameraIndex].drawn && visitState[cameraIndex] == 0)
            FindLoopReads(plans, viewOfTarget, cameraIndex, visitState, loopReads);
    }
    for (const auto& [readerIndex, targetId] : loopReads)
    {
        ViewPlan& reader = plans[readerIndex];
        std::erase(reader.readIds, targetId);
        reader.previousReadIds.push_back(targetId);
        plans[viewOfTarget.at(targetId)].needsHistory = true; // its target keeps last frame's copy
    }
    return plans;
}
}
