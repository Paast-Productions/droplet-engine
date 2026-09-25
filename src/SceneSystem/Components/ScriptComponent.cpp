#include "ScriptComponent.hpp"

using namespace Droplet::Scene;

ScriptComponent::ScriptComponent(const std::string &p_scriptPath)
    : m_scriptPath((p_scriptPath))
{
}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{
    //TODO: Lua script update will be implemented later.
}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}