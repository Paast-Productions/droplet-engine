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

void ScriptManager::Start()
{
	for (auto& instance : m_scriptInstances)
	{
		instance->onStart();
	}
}

void ScriptManager::Update(float p_deltaTime)
{
	for (auto& instance : m_scriptInstances)
	{
		instance->onUpdate(p_deltaTime);
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
	std::unique_ptr<ScriptInstance> scriptInstance = std::make_unique<ScriptInstance>(p_testNode, m_StateHandler, *loadResult, p_scriptFile);

	ScriptInstance* instance = scriptInstance.get();
	m_scriptInstances.push_back(std::move(scriptInstance));
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

	auto lastWriteTime = std::filesystem::last_write_time(scriptPath);

	if (!loadResult.valid())
	{
		sol::error error = loadResult;

		return false;
	}

	m_loadedScripts.emplace(p_scriptFile, LoadedScript{std::move(loadResult), lastWriteTime});

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

void ScriptManager::CheckForFileChanges()
{
	for (auto& [scriptFile, loadedScript] : m_loadedScripts)
	{
		if (m_HasScriptFileChanged(scriptFile))
		{
			std::print("{} changed!\n", scriptFile);
		}
	}
}
 

//Finds the script table for the parameter file, returns nullptr if the file isn't loaded
sol::load_result* ScriptManager::GetLoadedScript(const std::string& p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);
	
	if (it == m_loadedScripts.end())
	{
		return nullptr;
	}

	return &it->second.loadResult;
}

bool ScriptManager::m_Initialize()
{
	return false;
}

bool ScriptManager::m_HasScriptFileChanged(const std::string& p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);
	if (it == m_loadedScripts.end())
	{
		std::print("Script not found\n");
		return false;
	}

	std::filesystem::path scriptPath = std::filesystem::current_path() / ".." / ".." / ".." / "src" / "TestScripts" / p_scriptFile;
	
	auto currentWriteTime = std::filesystem::last_write_time(scriptPath);

	return currentWriteTime != it->second.lastWriteTime;
}
