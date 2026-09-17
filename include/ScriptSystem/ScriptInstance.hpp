#pragma once

//#include <utility> might be needed include this if you get errors
#include <string>
#include "LuaStateHandler.hpp"

/**
* The ScriptInstance class represents one scrips and its environment, it allows the engine to execute 
* lua functions. Functions like onStart and onUpdate will always exist, but the function "call" should 
* be able to call any function by name that has been populated in the scripts environment.
*/
class ScriptInstance
{
public:
	ScriptInstance(LuaStateHandler& p_stateHandler, sol::load_result& p_script, const std::string& p_scriptPath);
	~ScriptInstance() = default; 

	template<typename... Args>
	sol::protected_function_result call( 
		std::string_view functionName, 
		Args&&... args);

	void onStart();
	void onUpdate(float deltatime);
	const std::string getScriptPath();

private:
	LuaStateHandler& m_stateHandler;

	sol::environment m_environment;

	std::string m_scriptPath;

	sol::protected_function m_onStart;
	sol::protected_function m_onUpdate;
};

/**
* The call function should be able to call an arbitrary function inside the lua environment.
* All you need is the function name and potential arguments.
* 
* If call returns an error it should tell the logger, then do nothing.
*/
template<typename ...Args>
inline sol::protected_function_result ScriptInstance::call(std::string_view functionName, Args && ...args)
{
	sol::protected_function_result function = m_environment[std::string(functionName)];

	if (!function.valid())
	{
		//Function does not exist or is not callable
		return sol::protected_function_result();
	}
	return function(std::forward<Args>(args)...);
}
