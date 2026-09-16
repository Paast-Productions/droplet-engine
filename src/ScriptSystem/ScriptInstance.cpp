#include "ScriptInstance.hpp"

ScriptInstance::ScriptInstance(LuaStateHandler& p_stateHandler, sol::load_result p_script, const std::string& p_scriptPath) :
	m_stateHandler(p_stateHandler),
	m_environment(m_stateHandler.GetState(), sol::create, m_stateHandler.GetState().globals()),
	m_scriptPath(p_scriptPath)
{
	p_script(m_environment);

	m_onStart = m_environment["onStart"];
	m_onUpdate = m_environment["onUpdate"];
	//Error handling??
}

void ScriptInstance::onStart()
{
}

void ScriptInstance::onUpdate(float deltatime)
{
}
