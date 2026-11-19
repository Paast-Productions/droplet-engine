/**
 * @file Extract.hpp
 * @brief Bridge::Extract: turns a Game::Scene into the Render::RenderWorld snapshot the renderer draws.
 * @details The one place that reads the Scene for rendering. Reads no assets.
 */
#pragma once

#include "Integration/Game/Scene.hpp"
#include "Integration/Render/RenderWorld.hpp"

#include <span>
#include <string>

namespace Bridge
{
    /// @brief A camera that is not part of the scene (the editor's): culled against the scene like the scene's
    ///        cameras, drawing into a target it declares itself. Never presented, no post-process.
    struct ExternalCamera
    {
        std::string name;
        Game::Transform transform;
        Game::CameraComponent camera;   ///< camera.output is not used: `target` is the output
        Common::RenderTargetDesc target;
    };

    /// @brief Extract a RenderWorld snapshot of the scene: the scene's views with their visible meshes, each view's
    /// render-target reads (the edges the render graph sorts on), and a view per external camera after them. Makes no
    /// GPU calls; Vulkan::Prepare turns the world into draw lists.
    Render::RenderWorld Extract(const Game::Scene& scene, std::span<const ExternalCamera> externalCameras = {});

    /// Prints all of a RenderWorld's views, what they see, and what they depend on
    void Describe(const Render::RenderWorld& world, const Game::Scene& scene);
}
