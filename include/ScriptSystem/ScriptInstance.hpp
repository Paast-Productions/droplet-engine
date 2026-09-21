#pragma once

//#include <utility> might be needed include this if you get errors
#include <string>
#include "LuaStateHandler.hpp"
#include "TestNode.hpp"

/// @brief The ScriptInstance class represents one scrips and its environment, it allows the engine to execute lua functions.
/// Functions like onStart and onUpdate will always exist, but the function "call" should be able to call any function 
/// by name that has been populated in the scripts environment.
class ScriptInstance
{
public:
	/// @brief The constructor creates and populates the lua environment using the parameters. It also saves some useful data.
	/// @param p_stateHandler owns the lua state
	/// @param p_script is necessary to populate the lua environment
	/// @param p_scriptPath is the script path
	ScriptInstance(TestNode* p_testNode, LuaStateHandler& p_stateHandler, sol::load_result& p_script, const std::string& p_scriptPath);
	~ScriptInstance() = default;

	/// @brief The call function should be able to call an arbitrary function inside the lua environment. 
	/// All you need is the function name and potential arguments.
	/// @tparam ...Args 
	/// @param functionName is the name of the lua function
	/// @param ...args is the arguments for the lua function
	/// @return potential sol errors
	template<typename... Args>
	sol::protected_function_result call( 
		std::string_view functionName, 
		Args&&... args);

	/// @brief The onStart function should always exist inside a lua script and it can be called from here.
	void onStart();

	/// @brief The onUpdate function should always exist inside a lua script and it can be called from here.
	/// @param deltatime is the time since last function call
	void onUpdate(float deltatime);

	/// @brief Just gets the script path
	/// @return a string containing the script path
	std::string getScriptPath();
private:
	TestNode* m_testNode;
	LuaStateHandler& m_stateHandler;

	sol::environment m_environment;

	std::string m_scriptPath;

	sol::protected_function m_onStart;
	sol::protected_function m_onUpdate;
};

template<typename ...Args>
inline sol::protected_function_result ScriptInstance::call(std::string_view functionName, Args && ...args)
{
	sol::protected_function function = m_environment[std::string(functionName)];

	if (!function.valid())
	{
		//Function does not exist or is not callable
		//Add to logger here
	}
	return function(std::forward<Args>(args)...);
}
