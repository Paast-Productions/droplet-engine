#include "ScriptInstance.hpp"

/**
* The constructor creates and populates the lua environment using the parameters. It also saves some useful data.
*/
ScriptInstance::ScriptInstance(LuaStateHandler& p_stateHandler, sol::load_result& p_script, const std::string& p_scriptPath) :
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
}

/**
* The onStart function should always exist inside a lua script and it can be called from here.
* 
* If onStart for some reason doesn't exists it should tell the logger and destroy the script.
*/
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

/**
* The onUpdate function should always exist inside a lua script and it can be called from here.
* 
* If onUpdate for some reason doesn't exists it should tell the logger and destroy the script.
*/
void ScriptInstance::onUpdate(float deltatime)
{
	if (m_onUpdate.valid())
	{
		sol::protected_function_result result = m_onUpdate(deltatime);

		if (!result.valid())
		{
			sol::error err = result;
			//Send to error logger when it exists
		}
	}
}

const std::string ScriptInstance::getScriptPath()
{
	return m_scriptPath;
}
