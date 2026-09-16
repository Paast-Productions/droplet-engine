#include "ScriptManager.hpp"

ScriptManager::ScriptManager()
{
	if (!m_Initialize())
	{
		// send error to logger manager
	}
}

bool ScriptManager::LoadScript()
{
	return false;
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
