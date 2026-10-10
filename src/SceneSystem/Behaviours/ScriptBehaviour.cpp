#include "ScriptBehaviour.hpp"
#include <print>

using namespace Droplet::Scene;

void ScriptBehaviour::Start()
{
}

void ScriptBehaviour::Update([[maybe_unused]] float p_deltaTime)
{

}

const std::string &ScriptBehaviour::GetScriptPath() const
{
	return m_scriptPath;
}

void ScriptBehaviour::SetScriptPath(const std::string &p_scriptPath)
{
	m_scriptPath = p_scriptPath;

	// HACK: Likely doesn't work
	// TODO: Fix this to ensure scripts can be loaded after construction
	auto &scriptSystem = ScriptSystem::Get();
	scriptSystem.SetScriptPath("../../../src/TestScripts");
	scriptSystem.CreateBehaviourScript(this, m_scriptPath);
	scriptSystem.ActivateBehaviourScript(this);
}

void ScriptBehaviour::DetachScript()
{
	ScriptSystem::Get().DetachBehaviourScript(this);
}

void ScriptBehaviour::ActivateScript()
{
	ScriptSystem::Get().ActivateBehaviourScript(this);
}

void ScriptBehaviour::DeactivateScript()
{
	ScriptSystem::Get().DeactivateBehaviourScript(this);
}

void ScriptBehaviour::RenderUIImpl()
{
	ScriptSystem::Get().Call(this, "RenderUI");
}
