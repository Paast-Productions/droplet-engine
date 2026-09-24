#include "ScriptSystem.hpp"
#include "LuaBindings.hpp"

#include <print>

using namespace Droplet;

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{ 
	LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}

ScriptSystem::~ScriptSystem()
{
}

void ScriptSystem::Start()
{
	m_scriptManager.Start();
}

void ScriptSystem::Update(float p_deltaTime)
{
	m_scriptManager.Update(p_deltaTime);
	m_scriptManager.CheckForFileChanges();
}


bool ScriptSystem::CreateScript(Scene::Component *p_sciptComponent, const std::string &p_scriptFile)
{
	bool result = m_scriptManager.CreateScript(p_sciptComponent, p_scriptFile);
	if (!result)
	{
		std::print("ScriptSystem: Failed to create script instance for file: {}\n", p_scriptFile);
		return false;
	}
	return true;
}


bool ScriptSystem::LoadScript(const std::string &p_scriptFile)
{
	return m_scriptManager.LoadScript(p_scriptFile);
}

bool ScriptSystem::UnloadScript(const std::string &p_scriptFile)
{
	return m_scriptManager.UnloadScript(p_scriptFile);
}

void ScriptSystem::ActivateScript(Scene::Component *p_scriptComponent)
{
	m_scriptManager.ActivateScript(p_scriptComponent);
}

void ScriptSystem::DeactivateScript(Scene::Component *p_scriptComponent)
{
	m_scriptManager.DeactivateScript(p_scriptComponent);
}

void ScriptSystem::DetachScript(Droplet::Scene::Component *p_scriptComponent)
{
	m_scriptManager.DetachScript(p_scriptComponent);
}

bool ScriptSystem::SetScriptPath(const std::string &p_directoryPath)
{
	return m_scriptManager.SetScriptDirectory(p_directoryPath);
}

void ScriptSystem::DetachAllInstancesToScript(const std::string &p_scriptPath)
{
	m_scriptManager.DetachAllInstancesToScript(p_scriptPath);
}
