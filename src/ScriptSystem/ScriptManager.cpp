#include "ScriptManager.hpp"

ScriptManager::ScriptManager(LuaStateHandler& p_statehandler) : m_StateHandler(p_statehandler)
{
	if (!m_Initialize())
	{
		// send error to logger manager
	}
}

bool ScriptManager::LoadScript(const std::string& p_scriptFile)
{
	return false;
}

ScriptInstance* ScriptManager::CreateScript(const std::string& p_scriptFile)
{
	return nullptr;
}

void ScriptManager::UnloadScript(const std::string& scriptFile)
{
}

bool ScriptManager::IsLoaded(const std::string& scriptFile)
{
	return false;
}

bool ScriptManager::ReloadScript(const std::string& scriptFile)
{
	return false;
}

sol::table& ScriptManager::getScript(const std::string& scriptFile)
{
	// TODO: insert return statement here
}

bool ScriptManager::m_Initialize()
{
	return false;
}
