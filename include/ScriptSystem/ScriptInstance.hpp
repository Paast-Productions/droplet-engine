#pragma once

#include <sol/sol.hpp>
#include <string>

class ScriptInstance
{
public:
	ScriptInstance(const sol::state_view& p_luaState, const std::string& p_scriptPath);
	~ScriptInstance();

	template<typename... Args>
	sol::protected_function_result call(
		const std::string& scriptPath, 
		std::string_view functionName, 
		Args&&... args);

	void onStart();
	void onUpdate(float deltatime);

private:
	sol::state_view m_luaState;
	sol::environment m_environment;

	std::string m_scriptPath;

	sol::protected_function m_onStart;
	sol::protected_function m_onUpdate;

	bool m_loaded = false;
};
