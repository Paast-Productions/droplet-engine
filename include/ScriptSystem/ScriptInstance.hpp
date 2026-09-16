#pragma once

#include <string>
#include "LuaStateHandler.hpp"

/// <summary>
/// This class represents one script and allows the engine execute it.
/// </summary>

class ScriptInstance
{
public:
	ScriptInstance(LuaStateHandler& p_stateHandler, sol::load_result p_script, const std::string& p_scriptPath);
	~ScriptInstance() = default;

	template<typename... Args>
	sol::protected_function_result call( 
		std::string_view functionName, 
		Args&&... args);

	void onStart();
	void onUpdate(float deltatime);

private:
	LuaStateHandler& m_stateHandler;

	//Antingen Environment eller table beroende på vad vi behöver :)
	sol::environment m_environment;

	std::string m_scriptPath;

	sol::protected_function m_onStart;
	sol::protected_function m_onUpdate;
};

template<typename ...Args>
inline sol::protected_function_result ScriptInstance::call(std::string_view functionName, Args && ...args)
{
	return sol::protected_function_result();
}
