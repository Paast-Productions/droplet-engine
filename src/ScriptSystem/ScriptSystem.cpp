#include "ScriptSystem.hpp"
//#include "LuaBindings.hpp"

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{
}

void ScriptSystem::Initialize()
{
	//LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}  

void ScriptSystem::Update(float p_deltaTime)
{
	m_scriptManager.Update(p_deltaTime);
}

void ScriptSystem::CreateScript([[maybe_unused]] TestNode* testNode, [[maybe_unused]] const std::string& p_scriptFile)
{
	// shut everything down... idk what else...
}

void ScriptSystem::DestroyScript([[maybe_unused]] const std::string& p_scriptFile)
{
	m_scriptManager.DestroyScript(p_scriptFile);
}


