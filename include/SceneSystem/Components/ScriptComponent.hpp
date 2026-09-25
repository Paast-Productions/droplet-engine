#pragma once

#include <SceneSystem/Component.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>

using namespace Droplet::Script;

namespace Droplet::Scene
{
    /// @brief Component that attaches a Lua script to a Node.
    ///
    /// ScriptComponent connects a scene Node to a Lua script. It stores the
    /// path to the associated script and forwards script lifecycle operations
    /// to the ScriptSystem.
    ///
    /// A Lua script attached through a ScriptComponent can implement behavior
    /// such as movement, animation, interaction, or other game-specific logic.
    /// The component participates in the Node lifecycle through its Start()
    /// and Update() functions.
    class ScriptComponent : public Component
    {
    public:
        /// @brief Creates a ScriptComponent for a Lua script.
        ///
        /// @param p_scriptPath Path to the Lua script that should be associated
        /// with this component.
        explicit ScriptComponent(const std::string &p_scriptPath);

        /// @brief Updates the associated Lua script.
        ///
        /// Called during the Node's update cycle and forwards the update to
        /// the scripting system.
        ///
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
        void Update(float p_deltaTime) override;

        /// @brief Starts the associated Lua script.
        ///
        /// Called when the component is started and forwards the start
        /// operation to the scripting system.
        void Start() override;

        /// @brief Gets the path of the associated Lua script.
        ///
        /// @return Reference to the stored script path.
        const std::string &GetScriptPath() const;

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

        /// @brief Calls a Lua function on the associated script.
        ///
        /// The function is looked up by name in the script environment and
        /// the provided arguments are forwarded to the Lua function.
        ///
        /// @tparam Args Types of the arguments passed to the Lua function.
        /// @param p_functionName Name of the Lua function to call.
        /// @param p_args Arguments to forward to the Lua function.
        /// @return Result of the protected Lua function call.
        template<typename... Args>
        sol::protected_function_result Call(
            const std::string &p_functionName,
            Args&&... p_args);

    private:
        /// @brief Path to the Lua script associated with this component.
        std::string m_scriptPath;
    };


    /// @brief Calls a Lua function on the script associated with the component.
    ///
    /// The call is forwarded to the global ScriptSystem, which locates the
    /// script instance associated with this component and invokes the requested
    /// Lua function.
    ///
    /// @tparam Args Types of the arguments passed to the Lua function.
    /// @param p_functionName Name of the Lua function to call.
    /// @param p_args Arguments to forward to the Lua function.
    /// @return Result of the protected Lua function call.
    template<typename ...Args>
    inline sol::protected_function_result ScriptComponent::Call(
        const std::string &p_functionName,
        Args&& ...p_args)
    {
        return ScriptSystem::Get().Call(
            this,
            p_functionName,
            std::forward<Args>(p_args)...);
    }

}