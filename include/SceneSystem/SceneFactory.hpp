#pragma once

#include <memory>
#include <string>

namespace Droplet::Scene
{
    class Scene;

    /// @brief Factory for creating Scene instances.
    ///
    /// Provides a centralized way to create Scenes based on their name.
    /// This allows Scene creation logic to be kept separate from the
    /// SceneManager and other systems.
    class SceneFactory
    {
    public:

        /// @brief Creates a Scene with the specified name.
        ///
        /// @param p_name Name of the Scene to create.
        /// @return A shared pointer to the newly created Scene.
        static std::shared_ptr<Scene> CreateScene(const std::string &p_name);
    };
}