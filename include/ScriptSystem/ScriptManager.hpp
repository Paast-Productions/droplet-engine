#pragma once

#include "LuaStateHandler.hpp"
#include "TestNode.hpp"
#include "ScriptInstance.hpp"

#include <vector>
#include <string>
#include <filesystem>

/// @brief Stores information about a loaded Lua script.
///
/// LoadedScript contains the compiled Lua chunk together with information
/// about the source file. The file timestamp is used to detect when the
/// script has been modified and needs to be reloaded.
struct LoadedScript
{
	/// @brief Compiled Lua script.
	sol::load_result loadResult;

	/// @brief Path to the source Lua script.
	std::filesystem::path scriptPath;

	/// @brief Last recorded modification time of the source script.
	std::filesystem::file_time_type lastWriteTime;
};

/// @brief Manages Lua scripts and their relationships with script components.
///
/// ScriptManager is responsible for loading, unloading, reloading, and
/// retrieving Lua scripts. It also manages the relationship between
/// TestNode components and their ScriptInstance objects.
///
/// ScriptManager owns the created ScriptInstance objects and keeps track of
/// which scripts are currently active. It uses the LuaStateHandler provided
/// by the ScriptSystem to interact with the Lua state.
class ScriptManager
{
public:
	/// @brief Constructs a ScriptManager
	/// The LuaStateHandler is provided by the ScriptSystem and is used by the
	/// manager when loading and executing Lua scripts
	/// @param p_luaState Lua state handler used by the script manager.
	ScriptManager(LuaStateHandler& p_luaState);

	/// @brief Destroys the ScriptManager.
	~ScriptManager() = default;

	/// @brief Starts all active script instances.
	/// This should be called once the scripts are allowed to be active.
	void Start();

	/// @brief Updates all active script instances.
	/// This should be called once per frame to allow active scripts to perform
	/// their update logic.
	/// @param p_deltaTime Time elapsed since the previous update, in seconds.
	void Update(float p_deltaTime);

	/// @brief Calls a Lua function on a script attached to a component.
	/// The function is looked up through the relationship between the
	/// ScriptComponent and its ScriptInstance. The supplied arguments are forwarded
	/// to the Lua function.
	/// @tparam Args Types of the arguments passed to the Lua function.
	/// @param p_scriptComponent Component whose attached script should receive
	/// the function call.
	/// @param p_functionName Name of the Lua function to call.
	/// @param p_args Arguments to forward to the Lua function.
	/// @return Result of the protected Lua function call.
	template<typename... Args>
	sol::protected_function_result Call(
		TestNode* p_scriptComponent,
		std::string_view p_functionName,
		Args&&... p_args);

	/// @brief Creates a script instance and associates it with a component.
	/// The created ScriptInstance establishes the relationship between the
	/// specified component and the requested Lua script.
	/// @param p_scriptComponent Component that will own the script instance.
	/// @param p_scriptFile Path or name of the Lua script to associate with the component.
	/// @return Pointer to the created ScriptInstance.
	ScriptInstance* CreateScript(TestNode* p_scriptComponent, const std::string& p_scriptFile);

	/// @brief Detaches the script instance from a component.
	/// This removes the relationship between the specified component and its
	/// associated ScriptInstance.
	/// @param p_scriptComponent Component from which the script should be
	/// detached.
	void DetachScript(TestNode* p_scriptComponent);

	/// @brief Destroys a script instance.
	/// Removes the specified script instance and its associated relationships.
	/// @param p_scriptInstance Script instance to destroy.
	void DestroyScript(ScriptInstance* p_scriptInstance);

	/// @brief Loads a Lua script into the scripting system
	/// If the script is already loaded, the existing loaded script is kept
	/// rather than loading it again
	/// @param p_scriptFile Path or name of the Lua script to load.
	/// @return True if the script was successfully loaded or was already
	/// loaded, otherwise false.
	bool LoadScript(const std::string& p_scriptFile);

	/// @brief Unloads a previously loaded Lua script
	/// @param p_scriptFile Path or name of the Lua script to unload.
	/// @return True if the script was successfully unloaded, otherwise false.
	bool UnloadScript(const std::string& p_scriptFile);

	/// @brief Checks whether a Lua script is currently loaded
	/// @param p_scriptFile Path or name of the Lua script to check.
	/// @return True if the script is loaded, otherwise false.
	bool IsLoaded(const std::string& p_scriptFile);

	/// @brief Reloads a Lua script from its source file
	/// The existing loaded version is replaced with the current version of
	/// the script on disk
	/// @param p_scriptFile Path or name of the Lua script to reload.
	/// @return True if the script was successfully reloaded, otherwise false.
	bool ReloadScript(const std::string& p_scriptFile);

	/// @brief Checks loaded scripts for changes to their source files
	/// Scripts whose source files have been modified since they were loaded
	/// can be reloaded by the script manager.
	void CheckForFileChanges();

	/// @brief Retrieves a loaded Lua script
	/// @param p_scriptFile Path or name of the loaded Lua script.
	/// @return Pointer to the loaded script result, or nullptr if the script
	/// is not loaded.
	sol::load_result* GetLoadedScript(const std::string& p_scriptFile);

	/// @brief Activates a script component
	/// An activated script is added to the collection of scripts that are updated each frame
	/// @param p_scriptComponent Component whose script should be activated.
	void ActivateScript(TestNode* p_scriptComponent);

	/// @brief Deactivates a script component
	/// A deactivated script is removed from the collection of scripts that are updated each frame
	/// @param p_scriptComponent Component whose script should be deactivated.
	void DeactivateScript(TestNode* p_scriptComponent);

private:
	/// @brief Finds the path to a Lua script
	/// Searches for a script using its file name and returns the path to the corresponding file
	/// @param p_scriptFile Name of the script file to find.
	/// @return Path to the script file.
	std::filesystem::path FindScript(const std::string& p_scriptFile);

	/// @brief Destroys a script instance internally
	/// @param p_scriptInstance Script instance to destroy.
	void DestroyInstance(ScriptInstance* p_scriptInstance);

	/// @brief Checks whether a script file has been modified
	/// @param p_scriptFile Path to the Lua script file.
	/// @return True if the file has changed since it was last loaded,
	/// otherwise false.
	bool HasScriptFileChanged(const std::string& p_scriptFile);

private:
	/// @brief Reference to the Lua state used by the script manager
	/// The LuaStateHandler is owned by the ScriptSystem.
	LuaStateHandler& m_StateHandler;

	/// @brief Owns all script instances created by the manager.
	std::vector<std::unique_ptr<ScriptInstance>> m_scriptInstances;

	/// @brief Maps script components to their associated script instances
	/// This relationship allows the manager to find the ScriptInstance
	/// associated with a particular TestNode.
	std::unordered_map<TestNode*, ScriptInstance*> m_scripts;

	/// @brief Stores all Lua scripts that have been loaded
	/// The script path and modification time are stored together with the
	/// compiled Lua chunk to support script lookup and hot reloading.
	std::unordered_map<std::string, LoadedScript> m_loadedScripts;

	/// @brief Stores all currently active script instances
	/// Only active scripts are processed during the update loop.
	std::vector<ScriptInstance*> m_activeScripts;
};

/// @brief Calls a Lua function on the script attached to a component
/// The component is used to find its associated ScriptInstance. If the
/// component is null or does not have an attached script, an empty result
/// is returned
/// @tparam Args Types of the arguments passed to the Lua function.
/// @param p_scriptComponent Component whose script should receive the call.
/// @param p_functionName Name of the Lua function to call.
/// @param p_args Arguments to forward to the Lua function.
/// @return Result of the protected Lua function call.
template <typename... Args>
inline sol::protected_function_result ScriptManager::Call(
	TestNode* p_scriptComponent,
	std::string_view p_functionName,
	Args&&... p_args)
{
	if (p_scriptComponent == nullptr)
	{
		// TODO: Send error to logging manager instead of sending a nullptr
		return {};
	}

	std::unordered_map<TestNode*, ScriptInstance*>::iterator it =
		m_scripts.find(p_scriptComponent);

	if (it == m_scripts.end())
	{
		//TODO: attach component to script
		return {};
	}

	return it->second->Call(p_functionName, std::forward<Args>(p_args)...);
}