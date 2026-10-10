#pragma once
#include "SceneManager.hpp"
#include <SceneSystem/BehaviourFactory.hpp>
#include <json/json.hpp>

namespace Droplet::Scene
{
    /// @brief Base class for game-level behaviours.
    ///
    /// Behaviour provides a common interface for logic that is attached to
    /// the game.
    /// The main distinction is that Components are
    /// attached to Nodes, while Behaviours are attached to the game and can
    /// operate on the scene through the SceneManager.
    ///
    /// Behaviours can be derived from to implement game-level systems or logic
    /// that should not belong to a specific Node or Component.
    /// 
	/// As behaviours are not serialized with scenes, they are not automatically
    /// suited for tasks that require persistent data across runtimes.
    class Behaviour
    {
    public:
		Behaviour() = default;
        ~Behaviour() = default;

        /// @brief Gets the type name of the behaviour. 
        /// 
        /// This function must be overridden by derived classes to return the correct type name, 
        /// as it is used for serialization and identification of behaviour types.
        /// @return The type name of the behaviour.
        virtual std::string_view GetTypeName() = 0;

        /// @brief Called when the Behaviour is started.
        ///
        /// Derived behaviours can override this function to perform
        /// initialization that should occur when the game starts.
        virtual void Start() {}

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

        /// @brief Serializes the behaviour to a JSON object.
        /// @return A JSON object representing the behaviour's state.
        [[nodiscard]] nlohmann::json Serialize();

        /// @brief Deserializes the behaviour from a JSON object.
        /// @param p_behJson A JSON object containing the behaviour's state.
        void Deserialize(const nlohmann::json p_behJson);

    protected:
        /// @brief Internal rendering function for the behaviour's UI.
        /// 
        /// Overloaded by derived behaviours to implement their own UI rendering logic.
        virtual void RenderUIImpl() {}

        /// @brief Internal serialization function for the behaviour's state.
        /// 
        /// Overloaded by derived behaviours to implement their own serialization logic.
        /// @return A JSON object representing the behaviour's state.
        [[nodiscard]] virtual nlohmann::json SerializeImpl() { return {}; }

        /// @brief Internal deserialization function for the behaviour's state.
        /// 
        /// Overloaded by derived behaviours to implement their own deserialization logic.
        /// @param p_behJson A JSON object containing the behaviour's state.
        virtual void DeserializeImpl([[maybe_unused]] const nlohmann::json p_behJson) {}

    private:
        /// @brief SceneManager associated with this Behaviour.
        ///
        /// The SceneManager is not owned by the Behaviour. It provides access
        /// to the current scene and its Nodes.
        SceneManager *m_sceneManager = nullptr;
    };
}
