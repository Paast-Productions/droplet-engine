#include "ScriptSystem.hpp"

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{
}

void ScriptSystem::Initialize()
{
	LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}

void ScriptSystem::Update(float p_deltaTime)
{
	m_scriptManager.Update(p_deltaTime);
}

void ScriptSystem::CreateScript(TestNode* testNode, const std::string& p_scriptFile)
{
	// shut everything down... idk what else...
}

void ScriptSystem::CreateScript(const std::string& p_scriptFile)
{
	m_scriptManager.CreateScript(p_scriptFile);
}

void ScriptSystem::DestroyScript(const std::string& p_scriptFile)
{
	m_scriptManager.DestroyScript(p_scriptFile);
}


