#include "ScriptManager.hpp"

#include <iostream>
#include <print>

ScriptManager::ScriptManager(LuaStateHandler& p_statehandler) : m_StateHandler(p_statehandler)
{
}

void ScriptManager::Start()
{
	for (auto &instance : m_activeScripts)
	{
		instance->OnStart();
	}
}

void ScriptManager::Update(float p_deltaTime)
{
	for (auto &instance : m_activeScripts)
	{
		instance->OnUpdate(p_deltaTime);
	}
}

bool ScriptManager::CreateScript( TestNode *p_scriptComponent, const std::string &p_scriptFile)
{
	if (!IsLoaded(p_scriptFile))
	{
		if (!LoadScript(p_scriptFile))
		{
			//The script was not loaded and failed to load
			std::print("failed to load script\n");
			return false;
		}
	}
	std::print("load script successfully\n");

	sol::load_result *loadResult = GetLoadedScript(p_scriptFile);
	if (loadResult == nullptr)
	{
		//Send to logging manager
		return false;
	}
	
	if (p_scriptComponent == nullptr)
	{
		//Send error
		return false;
	}

	std::unordered_map<TestNode*, ScriptInstance*>::iterator existing = m_scripts.find(p_scriptComponent);
	ScriptInstance *oldinstance = nullptr;

	if (existing != m_scripts.end())
	{
		oldinstance = existing->second;
	}
	// We want to own unique ptrs, but return a instance
	// the caller gets a non-owning pointer, the manager should own the scriptinstances (in my humble opinion)
	std::unique_ptr<ScriptInstance> scriptInstance = std::make_unique<ScriptInstance>(p_scriptComponent, m_StateHandler, *loadResult, p_scriptFile);

	ScriptInstance *instance = scriptInstance.get();

	if (oldinstance != nullptr)
	{
		DestroyInstance(oldinstance);
	}
	m_scriptInstances.push_back(std::move(scriptInstance));
	m_scripts[p_scriptComponent] = instance;
	return true;
}

void ScriptManager::DetachScript( TestNode *p_scriptComponent)
{
	if (p_scriptComponent == nullptr)
	{
		return;
	}
	std::unordered_map<TestNode*, ScriptInstance*>::iterator it = m_scripts.find(p_scriptComponent);

	if (it == m_scripts.end())
	{
		//Couldn't find send an error
		return;
	}
	
	ScriptInstance *instance = it->second;
	// Destroy it
	DestroyInstance(instance);
}

void ScriptManager::DetachAllInstancesToScript(const std::string& p_scritpfile)
{
	std::vector<ScriptInstance*> instanceToKill;

	for (const std::unique_ptr<ScriptInstance> &instance : m_scriptInstances)
	{
		if (instance->GetScriptPath() == p_scritpfile)
		{
			instanceToKill.push_back(instance.get());
		}
	}

	for (ScriptInstance *instance : instanceToKill)
	{
		DestroyInstance(instance);
	}
}

bool ScriptManager::LoadScript(const std::string &p_scriptFile)
{
	if (IsLoaded(p_scriptFile))
	{
		return true; // Script is already loaded
	}

	std::filesystem::path scriptPath = FindScript(p_scriptFile);
	if (p_scriptFile.empty())
	{
		// TODO: Add to error logger
		return false;
	}

	sol::load_result loadResult = m_StateHandler.GetState().load_file(scriptPath.string());
	if (!loadResult.valid())
	{
		sol::error error = loadResult;
		std::print("Failed to load '{}': {}\n", scriptPath.string(), error.what());

		return false;
	}

	std::error_code errorCode;
	auto lastWriteTime = std::filesystem::last_write_time(scriptPath, errorCode);

	if (errorCode)
	{
		std::print("Failed to get last write time for '{}': {}\n", scriptPath.string(), errorCode.message());
		return false;
	}

	if (!loadResult.valid())
	{
		sol::error error = loadResult;

		return false;
	}

	m_loadedScripts.emplace(p_scriptFile, LoadedScript{std::move(loadResult), scriptPath, lastWriteTime});

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
		if (scriptInstance->GetScriptPath() == p_scriptFile)
		{
			return false; // a instance is using this script file
		}
	}

	m_loadedScripts.erase(p_scriptFile);
	return true; 
}

bool ScriptManager::IsLoaded(const std::string& p_scriptFile)
{
	 
	return m_loadedScripts.find(p_scriptFile) != m_loadedScripts.end(); // Possibly change this to a for loop
}

bool ScriptManager::ReloadScript(const std::string &p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);

	if (it == m_loadedScripts.end())
	{
		std::print("Cannot reload script '{}': not loaded\n", p_scriptFile);
		return false;
	}

	LoadedScript& loadedScript = it->second;

	sol::load_result newLoadResult = m_StateHandler.GetState().load_file(loadedScript.scriptPath.string());

	if (!newLoadResult.valid())
	{
		sol::error error = newLoadResult;

		std::print("Failed to reload '{}': {}\n", p_scriptFile, error.what());

		return false;
	}

	std::error_code errorCode;

	auto newLastWriteTime = std::filesystem::last_write_time(loadedScript.scriptPath, errorCode);

	if (errorCode)
	{
		std::print("Failed to get write time for '{}': {}\n", loadedScript.scriptPath.string(), errorCode.message());

		return false;
	}

	loadedScript.loadResult = std::move(newLoadResult);
	loadedScript.lastWriteTime = newLastWriteTime;

	for (auto& instance : m_scriptInstances)
	{
		if (instance->GetScriptPath() == p_scriptFile)
		{
			instance->Reload(loadedScript.loadResult); 
		}
	}

	std::print("{} has changed and reloaded!\n", loadedScript.scriptPath.string());

	return true;
}

void ScriptManager::CheckForFileChanges()
{
	for (auto& [scriptFile, loadedScript] : m_loadedScripts)
	{
		if (HasScriptFileChanged(scriptFile))
		{
			ReloadScript(scriptFile);
		}
	}
}

sol::load_result *ScriptManager::GetLoadedScript(const std::string& p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);
	
	if (it == m_loadedScripts.end())
	{
		return nullptr;
	}

	return &it->second.loadResult;
}

void ScriptManager::ActivateScript(TestNode *p_scriptComponent)
{
	if (p_scriptComponent == nullptr)
	{
		return; //No nullptr allowed
	}
	std::unordered_map<TestNode*, ScriptInstance*>::iterator it = m_scripts.find(p_scriptComponent);

	if (it == m_scripts.end())
	{
		return;
	}

	ScriptInstance *instance = it->second;

	if (std::find(m_activeScripts.begin(), m_activeScripts.end(), instance) != m_activeScripts.end())
	{
		return; // The instance is already activated
	}
	m_activeScripts.push_back(instance);
}

void ScriptManager::DeactivateScript(TestNode *p_scriptComponent)
{
	if (p_scriptComponent == nullptr)
	{
		return; //No nullptr allowed
	}
	std::unordered_map < TestNode*, ScriptInstance*>::iterator it = m_scripts.find(p_scriptComponent);

	if (it == m_scripts.end())
	{
		return;//no script found
	}

	ScriptInstance *instance = it->second;
	std::vector<ScriptInstance*>::iterator activeIt = std::find(m_activeScripts.begin(), m_activeScripts.end(), instance);
	if (activeIt == m_activeScripts.end())
	{
		return; // already inactive
	}

	*activeIt = m_activeScripts.back(); // overide the script we want to change with the last in the vector
	m_activeScripts.pop_back(); //we can now remove the last entry since it is a duplicate
}

bool ScriptManager::SetScriptDirectory(const std::string& p_directoryPath)
{
	std::filesystem::path scriptDirectory = std::filesystem::current_path() / p_directoryPath;

	if (!std::filesystem::exists(scriptDirectory) || !std::filesystem::is_directory(scriptDirectory))
	{
		return false;
	}
	m_scriptDirectoryPath = scriptDirectory;
	return true;
}


std::filesystem::path ScriptManager::FindScript(const std::string& p_scriptFile)
{
	std::filesystem::path scriptDirectory =
		std::filesystem::current_path() / ".." / ".." / ".." / "src" / "TestScripts";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(m_scriptDirectoryPath))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

		if (entry.path().filename() == p_scriptFile)
		{
			return entry.path();
		}
	}
	return {};
}

void ScriptManager::DestroyInstance(ScriptInstance *p_scriptInstance)
{
	if (p_scriptInstance == nullptr)
	{
		//Send error to logging
		return;
	}

	std::vector<ScriptInstance*>::iterator activeIterator = std::find(m_activeScripts.begin(), m_activeScripts.end(), p_scriptInstance);

	if (activeIterator != m_activeScripts.end())
	{
		*activeIterator = m_activeScripts.back();
		m_activeScripts.pop_back();
	}

	for (std::unordered_map<TestNode*, ScriptInstance*>::iterator it = m_scripts.begin(); it != m_scripts.end();)
	{
		if (it->second == p_scriptInstance)
		{
			it = m_scripts.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (std::vector<std::unique_ptr<ScriptInstance>>::iterator it = m_scriptInstances.begin(); it != m_scriptInstances.end(); it++)
	{
		if (it->get() == p_scriptInstance)
		{
			m_scriptInstances.erase(it);
			return;
		}
	}
	//instance was not found in ownership vector?
	// TODO: Log the error to the manager
}

bool ScriptManager::HasScriptFileChanged(const std::string& p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);
	if (it == m_loadedScripts.end())
	{
		std::print("Script is not loaded: {}\n", p_scriptFile);
		return false;
	}

	const auto& LoadedScript = it->second;

	std::error_code errorCode;

	auto currentWriteTime = std::filesystem::last_write_time(LoadedScript.scriptPath, errorCode);

	if (errorCode)
	{
		std::print("Failed to get write time for '{}': {}\n", LoadedScript.scriptPath.string(), errorCode.message());
		
		return false;
	}

	return currentWriteTime != it->second.lastWriteTime;
}
