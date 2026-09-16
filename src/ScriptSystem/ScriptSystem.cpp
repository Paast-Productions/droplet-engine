#include "ScriptSystem.hpp"

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{
}

void ScriptSystem::Initialize()
{
	LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}

void ScriptSystem::Update(float deltaTime)
{
	m_scriptManager.Update(deltaTime);
}

void ScriptSystem::CreateScript(TestNode* testNode, const std::string& p_scriptFile)
{
	m_scriptManager.CreateScript(testNode, p_scriptFile);
}

void ScriptSystem::DestroyScript(const std::string& p_scriptFile)
{
	m_scriptManager.DestroyScript(p_scriptFile);
}


