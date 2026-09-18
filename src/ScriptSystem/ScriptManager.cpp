#include "ScriptManager.hpp"
#include <iostream>
#include <print>
#include <filesystem>

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

ScriptInstance* ScriptManager::CreateScript([[maybe_unused]] TestNode* p_testNode, const std::string& p_scriptFile)
{
	if (!IsLoaded(p_scriptFile))
	{
		if (!LoadScript(p_scriptFile))
		{
			//The script was not loaded and failed to load
			std::print("failed to load script\n");
			return nullptr;
		}
	}
	std::print("load script successfully\n");

	sol::load_result* loadResult = GetLoadedScript(p_scriptFile);
	if (loadResult == nullptr)
	{
		//Send to logging manager
		return nullptr;
	}
	
	// We want to own unique ptrs, but return a instance
	// the caller gets a non-owning pointer, the manager should own the scriptinstances (in my humble opinion)
	std::unique_ptr<ScriptInstance> scriptInstance = std::make_unique<ScriptInstance>(m_StateHandler, *loadResult, p_scriptFile);

	ScriptInstance* instance = scriptInstance.get();
	m_scriptInstances.push_back(std::move(scriptInstance));
	instance->onStart();
	return instance;
}

void ScriptManager::DestroyScript([[maybe_unused]] ScriptInstance* p_scriptInstance)
{
	if (p_scriptInstance == nullptr) // who sends a nullptr to be destroyed?? name:
	{
		return;
	}		

	for (std::vector<std::unique_ptr<ScriptInstance>>::iterator it = m_scriptInstances.begin(); it != m_scriptInstances.end(); it++) // it for iterator
	{
		if (it->get() == p_scriptInstance)
		{		
			m_scriptInstances.erase(it); //erase destroys the unique ptr
			return;												
		}																				
	}

	// instance was handled well if we reach this point
	// send error to logging manager here
}

//loads teh script, if the script is already loaded it will return true, if script fails to load then return false and an error should be sent to the logging manager
//Noteworthy is that if the functions returns true it either means, "loaded successfully" or "it was already loaded", to check if it loads correctly make sure it is not loaded beforehand then run  this function
bool ScriptManager::LoadScript(const std::string& p_scriptFile)
{
	if (IsLoaded(p_scriptFile))
	{
		return true; // Script is already loaded
	}

	std::filesystem::path scriptPath = std::filesystem::current_path() / ".." / ".." / ".." / "src" / "TestScripts" / p_scriptFile;
	sol::load_result loadResult = m_StateHandler.GetState().load_file(scriptPath.string());

	if (!loadResult.valid())
	{
		sol::error error = loadResult;

		return false;
	}

	m_loadedScripts.emplace(p_scriptFile, std::move(loadResult));

	return true;
}

bool ScriptManager::UnloadScript(const std::string& p_scriptFile)
{
	if (!IsLoaded(p_scriptFile))
	{

		return false; // cant unload something that isn't loaded
	}

	for (const std::unique_ptr<ScriptInstance>& scriptInstance : m_scriptInstances) // check every instance
	{
		if (scriptInstance->getScriptPath() == p_scriptFile)
		{
			return false; // a instance is using this script file
		}
	}

	m_loadedScripts.erase(p_scriptFile);
	return true; 
}


//Searches through the loadedscripts to see if a script is loaded, returns true if it is loaded
bool ScriptManager::IsLoaded([[maybe_unused]] const std::string& p_scriptFile)
{
	/*return 0;*/ 
	return m_loadedScripts.find(p_scriptFile) != m_loadedScripts.end(); // Possibly change this to a for loop
}

bool ScriptManager::ReloadScript([[maybe_unused]] const std::string& scriptFile)
{
	return false;
}


//Finds the script table for the parameter file, returns nullptr if the file isn't loaded
sol::load_result* ScriptManager::GetLoadedScript([[maybe_unused]] const std::string& p_scriptFile)
{
	std::unordered_map<std::string, sol::load_result>::iterator it = m_loadedScripts.find(p_scriptFile);
	
	if (it == m_loadedScripts.end())
	{
		return nullptr;
	}
	return &it->second;
	
	return nullptr;
}

bool ScriptManager::m_Initialize()
{
	return false;
}
