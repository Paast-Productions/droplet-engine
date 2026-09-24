#include "ScriptComponent.hpp"

#include <print>

using namespace Droplet::Scene;
using namespace Droplet::Script;

ScriptComponent::ScriptComponent(const std::string &p_scriptPath)
    : m_scriptPath((p_scriptPath))
{
}

void ScriptComponent::Start()
{

    auto &scriptSystem = ScriptSystem::Get();

    scriptSystem.CreateScript(this, m_scriptPath);
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
    ScriptSystem::Get().DetachScript(this);
}

void ScriptComponent::ActivateScript()
{
    ScriptSystem::Get().ActivateScript(this);
}

void ScriptComponent::DeactivateScript()
{
    ScriptSystem::Get().DeactivateScript(this);
}
