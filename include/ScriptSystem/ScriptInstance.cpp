#include "ScriptInstance.hpp"

ScriptInstance::ScriptInstance(sol::state_view p_luaState, const std::string& p_scriptPath)
{
	m_luaState = p_luaState;
	m_scriptPath = p_scriptPath;
}
