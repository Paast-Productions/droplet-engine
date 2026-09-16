#include "ScriptManager.hpp"

ScriptManager::ScriptManager(LuaStateHandler& p_statehandler) : m_StateHandler(p_statehandler)
{
	if (!m_Initialize())
	{
		// send error to logger manager
	}
}

void ScriptManager::Update(float p_deltaTime)
{
	for (auto& script : m_scriptInstances)
	{
		script->onUpdate(p_deltaTime);
	}
}

//loads teh script, if the script is already loaded it will return true, if script fails to load then return false and an error should be sent to the logging manager
//Noteworthy is that if the functions returns true it either means, "loaded successfully" or "it was already loaded", to check if it loads correctly make sure it is not loaded beforehand then run  this function
bool ScriptManager::LoadScript(const std::string& p_scriptFile)
{
	if (IsLoaded(p_scriptFile))
	{
		return true; // Script is already loaded
	}

	sol::load_result loadResult = m_StateHandler.GetState().load_file(p_scriptFile);

	if (!loadResult.valid())
	{
		sol::error error = loadResult;
		//send the error to the logger

		return false;
	}

	m_loadedScripts.emplace(p_scriptFile, loadResult);

	return true;
}

ScriptInstance* ScriptManager::CreateScript(TestNode* testNode, const std::string& p_scriptFile)
{
	
	if (IsLoaded(p_scriptFile))
	{
		if (!LoadScript(p_scriptFile))
		{
			//The script was not loaded and failed to load
			return nullptr;
		}
	}

	sol::load_result* loadResult = GetLoadedScript(p_scriptFile);
	if (loadResult == nullptr)
	{
		//Send to logging manager
		return nullptr;
	}
	return nullptr;
}

void ScriptManager::DestroyScript(const std::string& p_scriptFile)
{

}


void ScriptManager::UnloadScript(const std::string& scriptFile)
{
	if (IsLoaded(scriptFile))
	{
		// check if any instances uses this particular script?
	}
}


//Searches through the loadedscripts to see if a script is loaded, returns true if it is loaded
bool ScriptManager::IsLoaded(const std::string& p_scriptFile)
{
	return m_loadedScripts.find(p_scriptFile) != m_loadedScripts.end(); // Possibly change this to a for loop
}

bool ScriptManager::ReloadScript(const std::string& scriptFile)
{
	return false;
}


//Finds the script table for the parameter file, returns nullptr if the file isn't loaded
sol::load_result* ScriptManager::GetLoadedScript(const std::string& p_scriptFile)
{
	std::unordered_map<std::string, sol::load_result>::iterator it = m_loadedScripts.find(p_scriptFile);
	
	if (it == m_loadedScripts.end())
	{
		return nullptr;
	}
	return &it->second;
	
}

bool ScriptManager::m_Initialize()
{
	return false;
}
