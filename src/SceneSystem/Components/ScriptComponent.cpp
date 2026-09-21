#include "ScriptComponent.hpp"

#include <utility>

ScriptComponent::ScriptComponent(std::string p_scriptPath)
    : m_scriptPath(std::move(p_scriptPath))
{
}

void ScriptComponent::Initialize()
{
    //TODO: Lua script loading and initialization will be implemented later.
}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{
    //TODO: Lua script update will be implemented later.
}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}