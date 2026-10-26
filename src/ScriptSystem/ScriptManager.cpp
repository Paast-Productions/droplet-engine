#include "ScriptManager.hpp"
#include "GameInput.hpp"

#include <iostream>
#include <fstream>
#include <print>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <tracy/public/tracy/TracyLua.hpp>


using namespace Droplet;
using namespace Droplet::Script;

ScriptManager::ScriptManager(LuaStateHandler &p_statehandler) : m_StateHandler(p_statehandler)
{
}

void ScriptManager::StartComponent(Droplet::Scene::Component *p_scriptComponent)
{
	ZoneScoped;

	if (ScriptInstance *instance = GetComponentsScript(p_scriptComponent))
	{
		instance->OnStart();
	}
}

void Droplet::Script::ScriptManager::StartBehaviour(Droplet::Scene::Behaviour *p_scriptBehaviour)
{
	ZoneScoped;

	if (ScriptInstance *instance = GetBehaviourScript(p_scriptBehaviour))
	{
		instance->OnStart();
	}

}

void ScriptManager::UpdateComponent(Droplet::Scene::Component *p_scriptComponent, float p_deltaTime)
{
	ZoneScoped;

	if (ScriptInstance *instance = GetComponentsScript(p_scriptComponent))
	{
		instance->OnUpdate(p_deltaTime);
	}
}

void Droplet::Script::ScriptManager::UpdateBehaviour(Droplet::Scene::Behaviour *p_scriptBehaviour, float p_deltaTime)
{
	ZoneScoped;

	if (ScriptInstance *instance = GetBehaviourScript(p_scriptBehaviour))
	{
		instance->OnUpdate(p_deltaTime);
	}
}

void ScriptManager::CreateComponentScript(
	Scene::Component *p_scriptComponent,
	const std::string &p_scriptFile)
{
	ZoneScoped;

	if (p_scriptComponent == nullptr)
	{
		throw std::invalid_argument(
			"Script component is nullptr.");
	}

	if (!IsLoaded(p_scriptFile))
	{
		LoadScript(p_scriptFile);
	}

	LoadedScript *loadedScript = GetLoadedScriptInfo(p_scriptFile);
	if (loadedScript == nullptr)
	{
		throw std::invalid_argument(
			"LoadedScript not found: " + p_scriptFile);
	}

	// Load a fresh Lua chunk for this instance.
	sol::load_result instanceLoadResult =
		m_StateHandler.GetState().load_file(
			loadedScript->scriptPath.string());

	if (!instanceLoadResult.valid())
	{
		sol::error error = instanceLoadResult;
		throw std::runtime_error(
			"Failed to load script '" + p_scriptFile +
			"': " + error.what());
	}

	auto existing = m_componentScripts.find(p_scriptComponent);
	ScriptInstance *oldInstance = nullptr;

	if (existing != m_componentScripts.end())
	{
		oldInstance = existing->second;
	}

	GameInput &input = GameInput::Get();

	auto scriptInstance = std::make_unique<ScriptInstance>(
		p_scriptComponent->GetOwner().get(),
		input,
		m_StateHandler,
		instanceLoadResult,
		p_scriptFile);

	ScriptInstance *instance = scriptInstance.get();

	// Replace the previous instance, if one exists.
	if (oldInstance != nullptr)
	{
		DestroyInstance(oldInstance);
	}

	m_scriptInstances.push_back(std::move(scriptInstance));
	m_componentScripts[p_scriptComponent] = instance;
}

void ScriptManager::CreateBehaviourScript(
	Scene::Behaviour *p_scriptBehaviour,
	const std::string &p_scriptFile)
{
	ZoneScoped;

	if (!IsLoaded(p_scriptFile))
	{
		LoadScript(p_scriptFile);
	}

	LoadedScript *loadedScript = GetLoadedScriptInfo(p_scriptFile);
	if (loadedScript == nullptr)
	{
		throw std::invalid_argument(
			"Loaded script information was not found.");
	}

	if (p_scriptBehaviour == nullptr)
	{
		throw std::invalid_argument(
			"Script component is nullptr.");
	}

	auto existing = m_behaviourScripts.find(p_scriptBehaviour);
	ScriptInstance *oldInstance = nullptr;

	if (existing != m_behaviourScripts.end())
	{
		oldInstance = existing->second;
	}

	// Load a fresh Lua chunk for this instance.
	sol::load_result instanceLoadResult =
		m_StateHandler.GetState().load_file(
			loadedScript->scriptPath.string());

	if (!instanceLoadResult.valid())
	{
		sol::error error = instanceLoadResult;
		throw std::runtime_error(
			"Failed to load script '" + p_scriptFile +
			"': " + error.what());
	}

	GameInput &input = GameInput::Get();

	auto scriptInstance = std::make_unique<ScriptInstance>(
		nullptr,
		input,
		m_StateHandler,
		instanceLoadResult,
		p_scriptFile);

	ScriptInstance *instance = scriptInstance.get();

	// Replace the previous instance, if one exists.
	if (oldInstance != nullptr)
	{
		DestroyInstance(oldInstance);
	}

	m_scriptInstances.push_back(std::move(scriptInstance));
	m_behaviourScripts[p_scriptBehaviour] = instance;
}

void ScriptManager::DetachComponentScript(Scene::Component *p_scriptComponent)
{
	ZoneScoped;

	if (p_scriptComponent == nullptr)
	{
		return;
	}
	std::unordered_map<Scene::Component*, ScriptInstance*>::iterator it = m_componentScripts.find(p_scriptComponent);

	if (it == m_componentScripts.end())
	{
		//Couldn't find send an error
		return;
	}
	
	ScriptInstance *instance = it->second;
	// Destroy it
	DestroyInstance(instance);
}

void ScriptManager::DetachBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour)
{
	ZoneScoped;

	if (p_scriptBehaviour == nullptr)
	{
		return;
	}
	std::unordered_map<Scene::Behaviour *, ScriptInstance *>::iterator it = m_behaviourScripts.find(p_scriptBehaviour);

	if (it == m_behaviourScripts.end())
	{
		return;
	}

	ScriptInstance *instance = it->second;
	DestroyInstance(instance);
}

void ScriptManager::DetachAllInstancesToScript(const std::string& p_scritpfile)
{
	ZoneScoped;

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

void ScriptManager::LoadScript(const std::string &p_scriptFile)
{
	ZoneScoped;

	if (p_scriptFile.empty())
	{
		throw std::runtime_error("p_scriptFile is empty");
	}

	if (IsLoaded(p_scriptFile))
	{
		return;
	}

	const std::filesystem::path scriptPath = FindScript(p_scriptFile);

	if (scriptPath.empty())
	{
		throw std::runtime_error(
			"Could not find script: " + p_scriptFile);
	}

	sol::load_result loadResult;

#ifdef TRACY_ENABLE
	loadResult = m_StateHandler.GetState().load_file(scriptPath.string());
#else
	std::string fileContents;
	std::ifstream file(scriptPath, std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error(
			"Could not open script: " + scriptPath.string());
	}

	std::getline(file, fileContents, '\0');
	file.close();

	tracy::LuaRemove(fileContents.data());

	loadResult = m_StateHandler.GetState().load(
		fileContents,
		sol::detail::default_chunk_name(),
		sol::load_mode::text);
#endif

	if (!loadResult.valid())
	{
		sol::error error = loadResult;

		std::print(
			"Failed to load script '{}': {}\n",
			scriptPath.string(),
			error.what());

		throw std::runtime_error(
			"Failed to load script '" + p_scriptFile +
			"': " + error.what());
	}

	std::error_code errorCode;
	const auto lastWriteTime =
		std::filesystem::last_write_time(scriptPath, errorCode);

	if (errorCode)
	{
		throw std::runtime_error(
			"Failed to get last write time for '" +
			scriptPath.string() + "': " + errorCode.message());
	}

	m_loadedScripts.emplace(
		p_scriptFile,
		LoadedScript{ scriptPath, lastWriteTime });
}

void ScriptManager::UnloadScript(const std::string& p_scriptFile)
{
	ZoneScoped;

	if (!IsLoaded(p_scriptFile))
	{
		throw std::runtime_error("Trying to unload a script that is not loaded");
	}

	for (const std::unique_ptr<ScriptInstance>& scriptInstance : m_scriptInstances) // check every instance
	{
		if (scriptInstance->GetScriptPath() == p_scriptFile)
		{
			throw std::runtime_error("A instance is using this script file");
		}
	}

	m_loadedScripts.erase(p_scriptFile);
}

bool ScriptManager::IsLoaded(const std::string& p_scriptFile)
{
	return m_loadedScripts.find(p_scriptFile) != m_loadedScripts.end(); // Possibly change this to a for loop
}

void ScriptManager::ReloadScript(const std::string &p_scriptFile)
{
	ZoneScoped;

	auto it = m_loadedScripts.find(p_scriptFile);

	if (it == m_loadedScripts.end())
	{
		throw std::runtime_error("Cannot reload script");
	}

	LoadedScript& loadedScript = it->second;

	sol::load_result newLoadResult = m_StateHandler.GetState().load_file(loadedScript.scriptPath.string());

	if (!newLoadResult.valid())
	{
		sol::error error = newLoadResult;
		throw std::runtime_error("Failed to reload '" + p_scriptFile + " " + error.what());
	}

	std::error_code errorCode;

	auto newLastWriteTime = std::filesystem::last_write_time(loadedScript.scriptPath, errorCode);

	if (errorCode)
	{
		throw std::runtime_error("Failed to get write time for " + loadedScript.scriptPath.string() + " " + errorCode.message());
	}

	loadedScript.lastWriteTime = newLastWriteTime;

	for (auto& instance : m_scriptInstances)
	{
		if (instance->GetScriptPath() == p_scriptFile)
		{
			sol::load_result instanceLoadResult = m_StateHandler.GetState().load_file(loadedScript.scriptPath.string());

			if (!instanceLoadResult.valid())
			{
				sol::error error = instanceLoadResult;
				throw std::runtime_error(
						"Failed to reload script '" + p_scriptFile +
				"': " + error.what());
			}
			instance->Reload(instanceLoadResult);
			instance->OnStart();
		}
	}

	std::print("{} has changed and reloaded!\n", loadedScript.scriptPath.string());
}

void ScriptManager::CheckForFileChanges()
{
	ZoneScoped;

	for (auto& [scriptFile, loadedScript] : m_loadedScripts)
	{
		if (HasScriptFileChanged(scriptFile))
		{
			ReloadScript(scriptFile);
		}
	}
}

LoadedScript *Droplet::Script::ScriptManager::GetLoadedScriptInfo(const std::string &p_scriptFile)
{
	auto it = m_loadedScripts.find(p_scriptFile);
		
	if (it == m_loadedScripts.end())
	{
		return nullptr;
	}
	
	return &it->second;
}

void ScriptManager::SetScriptDirectory(const std::string& p_directoryPath)
{
	std::filesystem::path scriptDirectory = std::filesystem::current_path() / p_directoryPath;

	if (!std::filesystem::exists(scriptDirectory) || !std::filesystem::is_directory(scriptDirectory))
	{
		throw std::invalid_argument("ScriptDirectory can't be found or doesn't exists");
	}
	m_scriptDirectoryPath = scriptDirectory;
}

std::string Droplet::Script::ScriptManager::GetScriptDirectory()
{
	return m_scriptDirectoryPath.string();
}

std::filesystem::path ScriptManager::FindScript(const std::string &p_scriptFile)
{
	ZoneScoped;

	const std::filesystem::path requestedPath =
		std::filesystem::path(p_scriptFile).lexically_normal();

	if (requestedPath.empty())
	{
		return {};
	}

	for (const auto &entry :
		std::filesystem::recursive_directory_iterator(m_scriptDirectoryPath))
	{
		if (!entry.is_regular_file())
		{
			continue;
		}

		const std::filesystem::path relativePath = entry.path()
			.lexically_relative(m_scriptDirectoryPath)
			.lexically_normal();

		if (relativePath == requestedPath)
		{
			return entry.path();
		}

		if (!requestedPath.has_parent_path() &&
			relativePath.filename() == requestedPath.filename())
		{
			return entry.path();
		}
	}

	return {};
}

void ScriptManager::DestroyInstance(ScriptInstance *p_scriptInstance)
{
	ZoneScoped;

	if (p_scriptInstance == nullptr)
	{
		throw std::invalid_argument("p_scriptInstance is nullptr");
	}

	// Remove the component association.
	std::erase_if(
		m_componentScripts,
		[p_scriptInstance](const auto &p_entry)
		{
			return p_entry.second == p_scriptInstance;
		});

	// Remove the behaviour association.
	std::erase_if(
		m_behaviourScripts,
		[p_scriptInstance](const auto &p_entry)
		{
			return p_entry.second == p_scriptInstance;
		});

	// Destroy the owned instance.
	const auto it = std::find_if(
		m_scriptInstances.begin(),
		m_scriptInstances.end(),
		[p_scriptInstance](const auto &p_instance)
		{
			return p_instance.get() == p_scriptInstance;
		});

	if (it != m_scriptInstances.end())
	{
		m_scriptInstances.erase(it);
	}
}

bool ScriptManager::HasScriptFileChanged(const std::string& p_scriptFile)
{
	ZoneScoped;

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

ScriptInstance *Droplet::Script::ScriptManager::GetComponentsScript(Droplet::Scene::Component *p_scriptComponent)
{
	if (p_scriptComponent == nullptr)
	{
		return nullptr;
	}

	const auto it = m_componentScripts.find(p_scriptComponent);

	if (it == m_componentScripts.end())
	{
		return nullptr;
	}

	return it->second;
}

ScriptInstance *Droplet::Script::ScriptManager::GetBehaviourScript(Droplet::Scene::Behaviour *p_scriptBehaviour)
{
	if (p_scriptBehaviour == nullptr)
	{
		return nullptr;
	}

	const auto it = m_behaviourScripts.find(p_scriptBehaviour);

	if (it == m_behaviourScripts.end())
	{
		return nullptr;
	}

	return it->second;
}
