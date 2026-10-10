#include "ScriptSystem.hpp"
#include "LuaBindings.hpp"

#include <print>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet;
using namespace Droplet::Script;

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{
	ZoneScoped;

	LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}

ScriptSystem::~ScriptSystem()
{
}

void ScriptSystem::Start()
{
	ZoneScoped;

	m_scriptManager.Start();
}

void ScriptSystem::Update(float p_deltaTime)
{
	ZoneScoped;

	m_scriptManager.Update(p_deltaTime);
	m_scriptManager.CheckForFileChanges();
}

void ScriptSystem::CreateComponentScript(Scene::Component *p_sciptComponent, const std::string &p_scriptFile)
{
	m_scriptManager.CreateComponentScript(p_sciptComponent, p_scriptFile);
}

void ScriptSystem::CreateBehaviourScript(Scene::Behaviour *p_scriptBehaviour, const std::string &p_scriptFile)
{
	m_scriptManager.CreateBehaviourScript(p_scriptBehaviour, p_scriptFile);
}

void ScriptSystem::LoadScript(const std::string &p_scriptFile)
{
	m_scriptManager.LoadScript(p_scriptFile);
}

void ScriptSystem::UnloadScript(const std::string &p_scriptFile)
{
	m_scriptManager.UnloadScript(p_scriptFile);
}

void ScriptSystem::ActivateComponentScript(Scene::Component *p_scriptComponent)
{
	m_scriptManager.ActivateComponentScript(p_scriptComponent);
}

void ScriptSystem::ActivateBehaviourScript(Scene::Behaviour *p_scriptBehaviour)
{
	m_scriptManager.ActivateBehaviourScript(p_scriptBehaviour);
}

void ScriptSystem::DeactivateComponentScript(Scene::Component *p_scriptComponent)
{
	m_scriptManager.DeactivateComponentScript(p_scriptComponent);
}

void ScriptSystem::DeactivateBehaviourScript(Scene::Behaviour *p_scriptBehaviour)
{
	m_scriptManager.DeactivateBehaviourScript(p_scriptBehaviour);
}

void ScriptSystem::DetachComponentScript(Scene::Component *p_scriptComponent)
{
	m_scriptManager.DetachComponentScript(p_scriptComponent);
}

void ScriptSystem::DetachBehaviourScript(Scene::Behaviour *p_scriptBehaviour)
{
	m_scriptManager.DetachBehaviourScript(p_scriptBehaviour);
}

void ScriptSystem::SetScriptPath(const std::string &p_directoryPath)
{
	m_scriptManager.SetScriptDirectory(p_directoryPath);
}

void ScriptSystem::DetachAllInstancesToScript(const std::string &p_scriptPath)
{
	m_scriptManager.DetachAllInstancesToScript(p_scriptPath);
}
