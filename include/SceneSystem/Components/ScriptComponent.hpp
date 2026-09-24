#pragma once

#include <SceneSystem/Component.hpp>
#include <string>
#include <ScriptSystem/ScriptSystem.hpp>

/// @brief Component that attaches a Lua script to a Node.
///
/// An example of a ScriptComponent could be a Lua script that controls the behavior of a game character, 
/// such as movement, animations, and interactions with other objects in the scene. 
/// The ScriptComponent would load the Lua script from the specified path and execute it during the Node's update cycle.
/// 
class ScriptComponent : public Component
{
public:
    explicit ScriptComponent(const std::string &p_path);
    void Update(float p_deltaTime) override;
    const std::string &GetScriptPath() const;

    void DetachScript(ScriptComponent *p_scriptComponent);
private:

    std::string m_scriptPath;
};