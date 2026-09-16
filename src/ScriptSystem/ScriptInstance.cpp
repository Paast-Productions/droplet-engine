#include "ScriptInstance.hpp"

ScriptInstance::ScriptInstance(LuaStateHandler& p_stateHandler, sol::load_result p_script, const std::string& p_scriptPath) :
	m_stateHandler(p_stateHandler),
	m_environment(m_stateHandler.GetState(), sol::create, m_stateHandler.GetState().globals()),
	m_scriptPath(p_scriptPath)
{
	sol::protected_function scriptFunction = p_script;

	sol::protected_function_result functionResult = scriptFunction(m_environment);
	if (!functionResult.valid())
	{
		sol::error error = functionResult;
		//send to logging manager because this is an error (:

		return;
	}

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
