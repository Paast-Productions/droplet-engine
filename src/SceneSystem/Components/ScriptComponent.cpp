#include "ScriptComponent.hpp"

#include <print>

using namespace Droplet::Scene;

ScriptComponent::ScriptComponent(const std::string &p_scriptPath)
    : m_scriptPath((p_scriptPath))
{

    auto &scriptSystem = ScriptSystem::Get();
    
    scriptSystem.CreateScript(this, p_scriptPath);
    scriptSystem.ActivateScript(this);
}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{
    ScriptSystem::Get().Update(1.0);
    //TODO: Lua script update will be implemented later.
}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}

void ScriptComponent::DetachScript()
{
    //auto &ScriptSystem = ScriptSystem::Get();
    
}

bool ScriptComponent::ActivateScript()
{
    return false;
}

bool ScriptComponent::DeactivateScript()
{
    return false;
}
