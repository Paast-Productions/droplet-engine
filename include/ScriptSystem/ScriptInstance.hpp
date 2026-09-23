#pragma once

#include <string>
#include "LuaStateHandler.hpp"
#include "TestNode.hpp"

/// @brief Represents an instance of a Lua script and its execution environment.
/// A ScriptInstance associates a Lua script with a TestNode and provides an
/// isolated Lua environment in which the script is executed. It provides
/// functions for calling arbitrary Lua functions as well as the standard
/// onStart and onUpdate lifecycle functions.
class ScriptInstance
{
public:
	/// @brief Creates a script instance and initializes its Lua environment.
	/// The provided Lua script is loaded into the instance's environment and
	/// the associated TestNode and script path are stored for later use.
	/// @param p_testNode TestNode associated with this script instance.
	/// @param p_stateHandler Lua state handler used by the script instance.
	/// @param p_script Loaded Lua script used to populate the script environment.
	/// @param p_scriptPath Path to the Lua script.
	ScriptInstance(
		TestNode* p_testNode,
		LuaStateHandler& p_stateHandler,
		sol::load_result& p_script,
		const std::string& p_scriptPath);

	/// @brief Destroys the script instance.
	~ScriptInstance() = default;

	/// @brief Calls an arbitrary Lua function in the script environment.
	/// The function is looked up by name in the script's Lua environment.
	/// Arguments provided to this function are forwarded to the Lua function.
	/// @tparam Args Types of the arguments passed to the Lua function.
	/// @param functionName Name of the Lua function to call.
	/// @param args Arguments to forward to the Lua function.
	/// @return Result of the protected Lua function call.
	template<typename... Args>
	sol::protected_function_result Call(std::string_view functionName, Args&&... args);

	/// @brief Calls the script's onStart lifecycle function.
	/// The onStart function is intended to be called when the script instance
	/// begins execution.
	void OnStart();

	/// @brief Calls the script's onUpdate lifecycle function.
	/// The onUpdate function is intended to be called once per frame while
	/// the script is active.
	/// @param deltatime Time elapsed since the previous update, in seconds.
	void OnUpdate(float deltatime);

	/// @brief Reloads the script instance with a new Lua script.
	/// The script environment is updated using the provided loaded script.
	/// @param p_script Loaded Lua script used to reload the instance.
	/// @return True if the script was successfully reloaded, otherwise false.
	bool Reload(sol::load_result& p_script);

	/// @brief Gets the path of the Lua script associated with this instance.
	/// @return The path to the associated Lua script.
	std::string GetScriptPath();

private:
	/// @brief TestNode associated with this script instance.
	TestNode* m_testNode;

	/// @brief Reference to the Lua state used by the script instance.
	LuaStateHandler& m_stateHandler;

	/// @brief Lua environment in which this script instance is executed.
	/// Each ScriptInstance has its own environment, allowing multiple script
	/// instances to maintain separate Lua state and variables.
	sol::environment m_environment;

	/// @brief Path to the Lua script associated with this instance.
	std::string m_scriptPath;

	/// @brief Lua function used for the script's start lifecycle event.
	sol::protected_function m_onStart;

	/// @brief Lua function used for the script's update lifecycle event.
	sol::protected_function m_onUpdate;
};

/// @brief Calls a Lua function in the script instance's environment.
/// The function is looked up by name in the Lua environment. If the function
/// does not exist or is not callable, an empty result is returned.
/// @tparam Args Types of the arguments passed to the Lua function.
/// @param functionName Name of the Lua function to call.
/// @param args Arguments to forward to the Lua function.
/// @return Result of the protected Lua function call.
template<typename ...Args>
inline sol::protected_function_result ScriptInstance::Call(
	std::string_view functionName,
	Args&& ...args)
{
	sol::protected_function function =
		m_environment[std::string(functionName)];

	if (!function.valid())
	{
		//Function does not exist or is not callable
		//TODO: Add to logger here
	}



	return function(std::forward<Args>(args)...);
}