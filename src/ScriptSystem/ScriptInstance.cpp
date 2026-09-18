#include "ScriptInstance.hpp"
#include <print>

ScriptInstance::ScriptInstance(TestNode* p_testNode, LuaStateHandler& p_stateHandler, sol::load_result& p_script, const std::string& p_scriptPath) :
	m_testNode(p_testNode),
	m_stateHandler(p_stateHandler),
	m_environment(m_stateHandler.GetState(), sol::create, m_stateHandler.GetState().globals()),
	m_scriptPath(p_scriptPath)
{
	m_environment["self"] = m_testNode;

	sol::protected_function scriptFunction = p_script;
	
	sol::set_environment(m_environment, scriptFunction);

	sol::protected_function_result functionResult = scriptFunction();
	if (!functionResult.valid())
	{
		sol::error error = functionResult;
		std::print("Script error: {}\n", error.what());

		return;
	}

	m_onStart = m_environment["onStart"];
	m_onUpdate = m_environment["onUpdate"];
}

void ScriptInstance::onStart()
{
	if (m_onStart.valid())
	{
		sol::protected_function_result result = m_onStart();

		if (!result.valid())
		{
			sol::error err = result;
			//Send to error logger when it exists
		}
	}
}

void ScriptInstance::onUpdate(float deltatime)
{
	if (m_onUpdate.valid())
	{
		sol::protected_function_result result = m_onUpdate(deltatime);

		if (!result.valid())
		{
			sol::error err = result;
			std::print("ScriptInstance: Error in onUpdate for script {}: {}\n", m_scriptPath, err.what());
			//Send to error logger when it exists
		}
	}
}

std::string ScriptInstance::getScriptPath()
{
	return m_scriptPath;
}
