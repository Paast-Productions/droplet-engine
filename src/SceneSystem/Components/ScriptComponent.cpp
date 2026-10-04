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
    //TODO: Should probably be moved somewhere more efficient, constructior or separate lode function,
    auto &scriptSystem = ScriptSystem::Get();

    scriptSystem.CreateComponentScript(this, m_scriptPath);
    scriptSystem.ActivateComponentScript(this);
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
    ScriptSystem::Get().DetachComponentScript(this);
}

void ScriptComponent::ActivateScript()
{
    ScriptSystem::Get().ActivateComponentScript(this);
}

void ScriptComponent::DeactivateScript()
{
    ScriptSystem::Get().DeactivateComponentScript(this);
}

void Droplet::Scene::ScriptComponent::RenderInternalUI()
{
}
