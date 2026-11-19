/**
 * @file Extract.hpp
 * @brief Bridge::Extract: turns a Game::Scene into the Render::RenderWorld snapshot the renderer draws.
 * @details The one place that reads the Scene for rendering. Reads no assets.
 */
#pragma once

#include "Integration/Game/Scene.hpp"
#include "Integration/Render/RenderWorld.hpp"

namespace Bridge
{
    /// @brief Extract a RenderWorld snapshot of the scene: the scene's views with their visible meshes, each view's
    /// render-target reads (the edges the render graph sorts on). Makes no GPU calls; Vulkan::Prepare turns the world into draw lists.
    Render::RenderWorld Extract(const Game::Scene& scene);

    /// Prints all of a RenderWorld's views, what they see, and what they depend on
    void Describe(const Render::RenderWorld& world, const Game::Scene& scene);
}
