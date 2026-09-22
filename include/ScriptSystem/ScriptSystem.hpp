#pragma once

#include "ScriptManager.hpp"
#include "LuaStateHandler.hpp"

/// @brief Provides the main interface for the engine's Lua scripting system.
/// ScriptSystem manages the lifecycle of scripts and coordinates between the
/// Lua state and the ScriptManager. It is responsible for starting and
/// updating the scripting system, loading and unloading script files, and
/// creating, activating, deactivating, and destroying script instances.
class ScriptSystem
{
public:
	/// @brief Constructs the script system.
	ScriptSystem();

	/// @brief Destroy the script system.
	~ScriptSystem() = default;

	/// @brief Starts the script script system.
	/// This should be called once the scene starts with the script system.
	void Start();

	/// @brief Updates the scripting system.
	/// This should be called once per frame to allow active scripts to perform
	/// their per-frame update logic.
	/// @param p_deltaTime Time elapsed since the previous update, in seconds.
	void Update(float p_deltaTime);

	/// @brief Loads a Lua script from a file.
	/// @param p_scriptFile Path to the Lua script file.
	/// @return True if the script was loaded successfully, otherwise false.
	bool LoadScript(const std::string& p_scriptFile);

	/// @brief Unloads a previously loaded Lua script.
	/// @param p_scriptFile Path to the Lua script file.
	/// @return True if the script was unloaded successfully, otherwise false.
	bool UnloadScript(const std::string& p_scriptFile);

	/// @brief Activates a script component.
	/// An activated script component is allowed to participate in the scripting
	/// system's update and execution flow.
	/// @param p_scriptComponent Script component to activate.
	void ActivateScript(TestNode* p_scriptComponent);

	/// @brief Deactivates a script component.
	/// A deactivated script component will no longer participate in the
	/// scripting system's update and execution flow.
	/// @param p_scriptComponent Script component to deactivate.
	void DeactivateScript(TestNode* p_scriptComponent);

	/// @brief Creates a script instance for a component.
	/// @param testNode Node that will own the script instance.
	/// @param p_scriptFile Path to the Lua script file associated with the instance.
	/// @return Pointer to the newly created script instance.
	ScriptInstance* CreateScript(TestNode* p_scriptComponent, const std::string& p_scriptFile);

	/// @brief Destroys a script instance.
	/// @param p_scriptInstance Script instance to destroy.
	void DestroyScript(ScriptInstance* p_scriptInstance);

	/// @brief Calls a Lua function on a script component.
	/// The supplied arguments are forwarded to the Lua function. The result
	/// contains the status and return values of the protected Lua function call.
	/// @tparam Args Types of the arguments passed to the Lua function.
	/// @param p_scriptComponent Script component containing the function to call.
	/// @param p_functionName Name of the Lua function to call.
	/// @param p_args Arguments to forward to the Lua function.
	/// @return Result of the protected Lua function call.
	template<typename... Args>
	sol::protected_function_result Call(
		TestNode* p_scriptComponent, std::string_view p_functionName, Args&&... p_args);

private:
	/// @brief Handles the Lua state used by the scripting system.
	LuaStateHandler m_luaStateHandler;

	/// @brief Manages script instances and their interaction with Lua.
	ScriptManager m_scriptManager;
};

/// @brief Calls a Lua function through the ScriptManager.
/// @tparam Args Types of the arguments passed to the Lua function.
/// @param p_scriptComponent Script component containing the function to call.
/// @param p_functionName Name of the Lua function to call.
/// @param p_args Arguments to forward to the Lua function.
/// @return Result of the protected Lua function call.
template<typename... Args>
inline sol::protected_function_result ScriptSystem::Call(
	TestNode* p_scriptComponent, std::string_view p_functionName, Args&&... p_args)
{
	return m_scriptManager.Call(p_scriptComponent, p_functionName, std::forward<Args>(p_args)...);
}