#pragma once

#include <sol/sol.hpp>
#include <string>

class ScriptInstance
{
public:
	ScriptInstance(sol::state_view p_luaState, const std::string& p_scriptPath);
	~ScriptInstance();

	bool Load();

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
