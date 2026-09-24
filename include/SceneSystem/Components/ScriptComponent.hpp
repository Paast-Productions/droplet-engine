#pragma once

#include <SceneSystem/Component.hpp>
#include <ScriptSystem/ScriptSystem.hpp>
#include <string>



namespace Droplet::Scene
{
    /// @brief Component that attaches a Lua script to a Node.
    ///
    /// An example of a ScriptComponent could be a Lua script that controls the behavior of a game character, 
    /// such as movement, animations, and interactions with other objects in the scene. 
    /// The ScriptComponent would load the Lua script from the specified path and execute it during the Node's update cycle.
    /// 
    class ScriptComponent : public Component
    {
    public:
        explicit ScriptComponent(const std::string &p_scriptPath);
        void Update(float p_deltaTime) override;
        const std::string &GetScriptPath() const;
		void DetachScript();
    	void ActivateScript();
    	void DeactivateScript();

        template<typename... Args>
        sol::protected_function_result Call(const std::string &p_functionName, Args&&... p_args);
    private:

        std::string m_scriptPath;

    };

 


    template<typename ...Args>
    inline sol::protected_function_result ScriptComponent::Call(const std::string &p_functionName, Args && ...p_args)
    {
        return ScriptSystem::Get().Call(this, p_functionName, std::forward<Args>(p_args)...);
    }

}