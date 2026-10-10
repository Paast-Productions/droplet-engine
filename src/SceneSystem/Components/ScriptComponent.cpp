#include "ScriptComponent.hpp"

#include <print>

using namespace Droplet::Scene;
using namespace Droplet::Script;

void ScriptComponent::Start()
{

}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{
}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}

void ScriptComponent::SetScriptPath(const std::string &p_scriptPath)
{
    m_scriptPath = p_scriptPath;

    // HACK: Likely doesn't work
    // TODO: Fix this to ensure scripts can be loaded after construction
    auto &scriptSystem = ScriptSystem::Get();

    scriptSystem.CreateComponentScript(this, m_scriptPath);
    scriptSystem.ActivateComponentScript(this);
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

void ScriptComponent::RenderUIImpl()
{
    ScriptSystem::Get().Call(this, "RenderUI");
}

nlohmann::json ScriptComponent::SerializeImpl()
{
    nlohmann::json json;

    json["filepath"] = GetScriptPath();

    return json;
}

void ScriptComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json  p_compJson)
{
	// TODO: Implement deserialization logic for the script component.
}
