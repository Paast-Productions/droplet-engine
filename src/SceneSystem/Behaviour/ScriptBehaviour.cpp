#include "ScriptBehaviour.hpp"

Droplet::Scene::ScriptBehaviour::ScriptBehaviour(const std::string &p_scriptPath)
	: m_scriptPath((p_scriptPath))
{
	//auto &ScriptSystem = ScriptSystem::Get();
	//scriptSystem.CreateScript(this, m_scriptPath);
}

void Droplet::Scene::ScriptBehaviour::Start()
{

}

void Droplet::Scene::ScriptBehaviour::Update([[maybe_unused]] float p_deltaTime)
{
	ScriptSystem::Get().Update(1.0);
}

const std::string &Droplet::Scene::ScriptBehaviour::GetScriptPath() const
{
	return m_scriptPath;
}

void Droplet::Scene::ScriptBehaviour::DetachScript()
{
	//ScriptSystem::Get().DetachScript(this);
}

void Droplet::Scene::ScriptBehaviour::ActivateScript()
{
	//ScriptSystem::Get().ActivateScript(this);
}

void Droplet::Scene::ScriptBehaviour::DeactivateScript()
{
	//ScriptSystem::Get().DeactivateScript(this);
}
