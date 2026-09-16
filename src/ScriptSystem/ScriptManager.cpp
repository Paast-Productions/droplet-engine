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


	sol::protected_function scriptFunction = loadResult;

	sol::protected_function_result scriptFunctionResult = scriptFunction();

	if (!scriptFunctionResult.valid())
	{
		sol::error error = scriptFunctionResult;
		//Yet again send the error to the logger

		return false;
	}

	sol::object scriptObject = scriptFunctionResult.get<sol::object>();
	if (!scriptObject.is<sol::table>())
	{
		//Yet another error the script did not return a table
		return false;
	}

	m_loadedScripts.emplace(p_scriptFile, scriptObject.as<sol::table>());

	return true;
}

ScriptInstance* ScriptManager::CreateScript(const std::string& p_scriptFile)
{
	return nullptr;
	if (IsLoaded(p_scriptFile))
	{
		 
	}
	else
	{

	}

}

void ScriptManager::DestroyScript(const std::string& p_scriptFile)
{

}


void ScriptManager::UnloadScript(const std::string& scriptFile)
{
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

sol::table& ScriptManager::GetScript(const std::string& p_scriptFile)
{

	//fast dirty solution fix later
	return m_loadedScripts.at(p_scriptFile);
	// TODO: insert return statement here
}

bool ScriptManager::m_Initialize()
{
	return false;
}
