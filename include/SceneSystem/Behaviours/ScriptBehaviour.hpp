#pragma once

#include <SceneSystem/Behaviour.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

using namespace Droplet::Script;

namespace Droplet::Scene
{
    /// @brief Behaviour that attaches a Lua script to game.
    ///
    /// ScriptBehaviour connects a scene a Lua script to the game. It stores the
    /// path to the associated script and forwards script lifecycle operations
    /// to the ScriptSystem.
    ///
    /// A Lua script attached through a ScriptBehaviour can implement behavior
    /// such as movement, animation, interaction, or other game-specific logic.
    /// The component participates in the Node lifecycle through its Start()
    /// and Update() functions.
    class ScriptBehaviour : public Behaviour
    {
    public:
        /// @brief Creates a ScriptBehaviour for a Lua script.
        ///
        /// @param p_scriptPath Path to the Lua script that should be associated
        /// with this component.
        explicit ScriptBehaviour(const std::string &p_scriptPath);

        /// @brief Starts the associated Lua script.
        ///
        /// Called when the component is started and forwards the start
        /// operation to the scripting system.
        void Start() override;

        /// @brief Updates the associated Lua script.
        ///
        /// Called during the Games update cycle and forwards the update to
        /// the scripting system.
        ///
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
        void Update(float p_deltaTime) override;

        /// @brief Gets the path of the associated Lua script.
        ///
        /// @return Reference to the stored script path.
        [[nodiscard]] const std::string &GetScriptPath() const;

        /// @brief Detaches the script from this component.
        ///
        /// Removes the relationship between this component and its associated
        /// script instance.
        void DetachScript();

        /// @brief Activates the script associated with this component.
        ///
        /// An activated script participates in the scripting system's update
        /// cycle.
        void ActivateScript();

        /// @brief Deactivates the script associated with this component.
        ///
        /// A deactivated script no longer participates in the scripting
        /// system's update cycle.
        void DeactivateScript();

	protected:
		/// @brief Internal rendering function for the component's UI.
		/// Overloaded by derived components to implement their own UI rendering logic.
		void RenderInternalUI() override;

    private:
        /// @brief Path to the Lua script associated with this component.
        std::string m_scriptPath;
    };

}