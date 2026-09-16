#include "ScriptInstance.hpp"

ScriptInstance::ScriptInstance(LuaStateHandler& p_stateHandler, const std::string& p_scriptPath) :
	m_stateHandler(p_stateHandler),
	m_scriptPath(p_scriptPath) 
{
}
