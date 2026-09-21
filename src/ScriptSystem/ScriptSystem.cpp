#include "ScriptSystem.hpp"
#include <print>

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{ 
	LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
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

bool ScriptSystem::LoadScript(const std::string& p_scriptFile)
{
	return m_scriptManager.LoadScript(p_scriptFile);
}

bool ScriptSystem::UnloadScript(const std::string& p_scriptFile)
{
	return m_scriptManager.UnloadScript(p_scriptFile);
}

ScriptInstance* ScriptSystem::CreateScript([[maybe_unused]] TestNode* p_testNode, [[maybe_unused]] const std::string& p_scriptFile)
{
	ScriptInstance* instance = m_scriptManager.CreateScript(p_testNode, p_scriptFile);
	if (instance == nullptr)
	{
		std::print("ScriptSystem: Failed to create script instance for file: {}\n", p_scriptFile);
		return nullptr;
	}
	return instance;
}

void ScriptSystem::DestroyScript([[maybe_unused]] ScriptInstance* p_scriptInstance)
{
	m_scriptManager.DestroyScript(p_scriptInstance);
}


