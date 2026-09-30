#include "ScriptBehaviour.hpp"
#include <print>

Droplet::Scene::ScriptBehaviour::ScriptBehaviour([[maybe_unused]]const std::string &p_scriptPath)
	: m_scriptPath((p_scriptPath))
{
	auto &scriptSystem = ScriptSystem::Get();
	std::print("ScriptPath: {}\n", p_scriptPath);
	scriptSystem.SetScriptPath("../../../src/TestScripts");
	scriptSystem.CreateBehaviourScript(this, m_scriptPath);
	scriptSystem.ActivateBehaviourScript(this);
}

void Droplet::Scene::ScriptBehaviour::Start()
{
}

void Droplet::Scene::ScriptBehaviour::Update([[maybe_unused]] float p_deltaTime)
{

}

const std::string &Droplet::Scene::ScriptBehaviour::GetScriptPath() const
{
	return m_scriptPath;
}

void Droplet::Scene::ScriptBehaviour::DetachScript()
{
	ScriptSystem::Get().DetachBehaviourScript(this);
}

void Droplet::Scene::ScriptBehaviour::ActivateScript()
{
	ScriptSystem::Get().ActivateBehaviourScript(this);
}

void Droplet::Scene::ScriptBehaviour::DeactivateScript()
{
	ScriptSystem::Get().DeactivateBehaviourScript(this);
}
