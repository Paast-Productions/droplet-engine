#pragma once

#include <sol/sol.hpp>
#include <string>
#include "LuaStateHandler.hpp"

class ScriptInstance
{
public:
	ScriptInstance(LuaStateHandler& p_stateHandler, const std::string& p_scriptPath);
	~ScriptInstance();

	template<typename... Args>
	sol::protected_function_result call( 
		std::string_view functionName, 
		Args&&... args);

	void onStart();
	void onUpdate(float deltatime);

private:
	LuaStateHandler& m_stateHandler;
	sol::environment m_environment;

	std::string m_scriptPath;

	sol::protected_function m_onStart;
	sol::protected_function m_onUpdate;

	bool m_loaded = false;
};
