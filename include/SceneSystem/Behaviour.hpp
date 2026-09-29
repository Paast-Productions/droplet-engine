#pragma once

#include "SceneManager.hpp"

namespace Droplet::Scene
{
    /// @brief Base class for game-level behaviours.
    ///
    /// Behaviour provides a common interface for logic that is attached to
    /// the game rather than to an individual Node.
    ///
    /// Similar to a Component, a Behaviour provides lifecycle functions such
    /// as Start() and Update(). The main distinction is that Components are
    /// attached to Nodes, while Behaviours are attached to the game and can
    /// operate on the scene through the SceneManager.
    ///
    /// Behaviours can be derived from to implement game-level systems or logic
    /// that should not belong to a specific Node or Component.
    class Behaviour
    {
    public:
        /// @brief Creates a Behaviour.
        Behaviour() = default;

        /// @brief Destroys the Behaviour.
        virtual ~Behaviour() = default;

        /// @brief Called when the Behaviour is started.
        ///
        /// Derived behaviours can override this function to perform
        /// initialization that should occur when the game starts.
        virtual void Start();

        /// @brief Updates the Behaviour.
        ///
        /// Called during the game's update cycle. Derived behaviours can
        /// override this function to perform game-level logic each frame.
        ///
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
        virtual void Update([[maybe_unused]] float p_deltaTime) {}

        /// @brief Sets the SceneManager used by the Behaviour.
        ///
        /// The SceneManager provides the Behaviour with access to the current
        /// scene and its scene-level functionality.
        ///
        /// @param sceneManager SceneManager associated with the game.
        void SetSceneManager(SceneManager *sceneManager);

    private:
        /// @brief SceneManager associated with this Behaviour.
        ///
        /// The SceneManager is not owned by the Behaviour. It provides access
        /// to the current scene and its Nodes.
        SceneManager *m_sceneManager = nullptr;
    };
}
