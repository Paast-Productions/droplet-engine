/**
 * @file Extract.cpp
 * @brief Bridge::Extract implementation: objects, views and their culling, render-target reads and loop breaking.
 */
#include "Integration/Bridge/Extract.hpp"


#include <algorithm>
#include <cstdint>
#include <print>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Bridge
{
    using Render::RenderObject;
    using Render::RenderView;
    using Render::RenderWorld;

    namespace
    {
        /// Copies the scene's description of target `id` into the world once. Returns false if the scene has none.
        bool CollectTarget(RenderWorld& world, const Game::Scene& scene, Guid id)
        {
            if (std::ranges::any_of(world.targets, [id](const Common::RenderTargetDesc& desc)
            {
                return desc.id == id;
            }))
                return true;

            const Common::RenderTargetDesc* desc = scene.FindTarget(id);
            if (!desc)
                return false;

            world.targets.push_back(*desc);
            return true;
        }

        /// @brief Depth-first search over "view reads the target of view"; collects the reads that lead back to a view
        ///        still on the search path (the edges that close a loop).
        void FindLoopReads(
            const RenderWorld& world,
            const std::unordered_map<Guid, std::uint32_t>& viewOfTarget,
            std::uint32_t viewIndex,
            std::vector<std::uint8_t>& visitState, // 0 not visited, 1 on the search path, 2 done
            std::vector<std::pair<std::uint32_t, Guid>>& loopReads)
        {
            visitState[viewIndex] = 1;
            for (const Guid readId : world.views[viewIndex].readIds)
            {
                const auto ownerIt = viewOfTarget.find(readId);
                if (ownerIt == viewOfTarget.end())
                    continue; // a target no view draws: nothing to order
                const std::uint32_t ownerIndex = ownerIt->second;
                if (visitState[ownerIndex] == 1)
                    loopReads.emplace_back(viewIndex, readId);
                else if (visitState[ownerIndex] == 0)
                    FindLoopReads(world, viewOfTarget, ownerIndex, visitState, loopReads);
            }
            visitState[viewIndex] = 2;
        }
    }

    namespace
    {
        /// @brief Turns every read that closes a loop between views into a previous-frame read (RenderView::previousReadIds,
        ///        and marks the read view's target for a history copy), so the views can be ordered. The views are
        ///        searched in declaration order.
        void BreakViewLoops(RenderWorld& world)
        {
            std::unordered_map<Guid, std::uint32_t> viewOfTarget;
            for (std::uint32_t viewIndex = 0; viewIndex < world.views.size(); ++viewIndex)
                viewOfTarget.emplace(world.views[viewIndex].targetId, viewIndex);

            // removing every edge that closes a loop in a depth-first search leaves no loop
            std::vector<std::uint8_t> visitState(world.views.size(), 0);
            std::vector<std::pair<std::uint32_t, Guid>> loopReads;
            for (std::uint32_t viewIndex = 0; viewIndex < world.views.size(); ++viewIndex)
            {
                if (visitState[viewIndex] == 0)
                    FindLoopReads(world, viewOfTarget, viewIndex, visitState, loopReads);
            }

            for (const auto& [readerIndex, targetId] : loopReads)
            {
                RenderView& reader = world.views[readerIndex];
                std::erase(reader.readIds, targetId);
                reader.previousReadIds.push_back(targetId);
                world.views[viewOfTarget.at(targetId)].needsHistory = true; // its target keeps last frame's copy
            }
        }
    }

    RenderWorld Extract(const Game::Scene& scene)
    {
        RenderWorld world;

        // 1. objects: every mesh node; the views below name them by node index
        constexpr std::uint32_t NO_OBJECT = 0xFFFFFFFFu;
        std::vector<std::uint32_t> objectOfNode(scene.nodes.size(), NO_OBJECT);
        for (std::uint32_t i = 0; i < scene.nodes.size(); ++i)
        {
            const auto& node = scene.nodes[i];
            if (!node.mesh)
                continue;

            RenderObject renderObject{
                .model = node.transform.Matrix(),
                .sourceNode = i,
                .meshId = node.mesh->mesh,
                .imageTexture = node.mesh->imageTexture,
                .shaderId = node.mesh->shader,
            };

            // a render target used as a texture: its description travels with the world. One the scene does not
            // declare keeps its id: Prepare cannot resolve it and draws the placeholder texture
            if (renderObject.imageTexture.kind == Common::ImageRef::Kind::RenderTarget)
                CollectTarget(world, scene, renderObject.imageTexture.id);

            objectOfNode[i] = static_cast<std::uint32_t>(world.objects.size());
            world.objects.emplace_back(renderObject);
        }

        // 2. views: the scene says which meshes each camera sees
        for (const Game::SceneView& sceneView : scene.CullViews())
        {
            const auto& node = scene.nodes[sceneView.cameraNode];

            if (!CollectTarget(world, scene, node.camera->output))
                continue;

            RenderView camera;
            camera.name = node.name;
            camera.targetId = node.camera->output;

            if (node.camera->present)
            {
                if (world.presentedId.IsValid())
                {
                    std::println("Extract: more than one camera set to present; {} is ignored.", node.name);
                }
                else
                {
                    world.presentedId = camera.targetId;
                }
            }

            const Game::CameraMatrices matrices = Game::MakeCameraMatrices(node);
            camera.view = matrices.view;
            camera.proj = matrices.proj;

            if (node.postProcess)
            {
                for (const Game::PostProcessComponent::Effect& effect : node.postProcess->effects)
                {
                    if (effect.enabled)
                        camera.effects.push_back({.effect = effect.effect, .amount = effect.amount});
                }
            }

            for (const std::uint32_t nodeIndex : sceneView.visibleNodes)
            {
                const std::uint32_t index = objectOfNode[nodeIndex];
                const auto& renderObject = world.objects[index];

                camera.visible.push_back(index);

                if (renderObject.imageTexture.kind != Common::ImageRef::Kind::RenderTarget)
                    continue;

                // a target the scene does not declare is not read (nothing renders it)
                const Guid sampledTarget = renderObject.imageTexture.id;
                const auto declaresSampledTarget = [sampledTarget](const Common::RenderTargetDesc& desc)
                {
                    return desc.id == sampledTarget;
                };
                if (std::ranges::none_of(world.targets, declaresSampledTarget))
                    continue;
                if (sampledTarget == camera.targetId)
                {
                    // the camera sees its own output: it reads last frame's copy (history)
                    camera.needsHistory = true;
                }
                else if (std::ranges::find(camera.readIds, sampledTarget) == camera.readIds.end())
                {
                    camera.readIds.push_back(sampledTarget);
                }
            }

            world.views.push_back(std::move(camera));
        }

        if (!world.presentedId.IsValid() && !world.views.empty())
            world.presentedId = world.views.back().targetId;

        BreakViewLoops(world);

        return world;
    }

    void Describe(const RenderWorld& world, const Game::Scene& scene)
    {
        const auto targetName = [&](Guid targetId)
        {
            for (const auto& view : world.views)
                if (view.targetId == targetId)
                    return view.name;
            return std::string("<target without a view>");
        };

        for (const auto& view : world.views)
        {
            std::println("View '{}'", view.name);
            for (const auto index : view.visible)
            {
                std::println("  sees  : {}", scene.nodes[world.objects[index].sourceNode].name);
            }
            for (const Guid readTarget : view.readIds)
            {
                std::println("  reads : output of '{}'", targetName(readTarget));
            }
            for (const Guid previousTarget : view.previousReadIds)
            {
                std::println("  reads : previous frame of '{}' (breaks a loop)", targetName(previousTarget));
            }
            if (view.needsHistory)
            {
                std::println("  keeps : a previous-frame copy (history)");
            }
            if (view.readIds.empty() && view.previousReadIds.empty() && !view.needsHistory)
            {
                std::println("  reads : nothing");
            }
            for (const Common::PostEffectDesc& effect : view.effects)
            {
                std::println("  effect: {}", Common::ToString(effect.effect));
            }
        }
    }
}
