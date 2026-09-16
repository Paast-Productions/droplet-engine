#include "ScriptSystem.hpp"

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler.GetState())
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

void ScriptSystem::Shutdown()
{
	m_scriptManager.Shutdown();
}

void ScriptSystem::CreateScript()
{
	m_scriptManager.CreateScript();
}

void ScriptSystem::DestroyScript()
{
	m_scriptManager.DestroyScript();
}


