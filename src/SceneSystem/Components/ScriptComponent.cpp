#include "ScriptComponent.hpp"

ScriptComponent::ScriptComponent([[maybe_unused]] const std::string &p_path)
{
    //ScriptSystem scriptsystem;
    auto &scriptSystem = ScriptSystem::Get();
    //scriptSystem.CreateScript(this, p_path);
}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{
    //TODO: Lua script update will be implemented later.
}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}

void ScriptComponent::DetachScript()
{
    auto &ScriptSystem = ScriptSystem::Get();
    
}

bool ScriptComponent::ActivateScript()
{
    return false;
}

bool ScriptComponent::DeactivateScript()
{
    return false;
}
