#include "ScriptInstance.hpp"
#include <print>

ScriptInstance::ScriptInstance(ScriptComponent *p_scriptComponent, LuaStateHandler &p_stateHandler, sol::load_result &p_script, const std::string &p_scriptPath) :
	m_scriptComponent(p_scriptComponent),
	m_stateHandler(p_stateHandler),
	m_environment(m_stateHandler.GetState(), sol::create, m_stateHandler.GetState().globals()),
	m_scriptPath(p_scriptPath)
{
	m_environment["self"] = m_scriptComponent->GetOwner();

	sol::protected_function scriptFunction = p_script;

	sol::set_environment(m_environment, scriptFunction);

	sol::protected_function_result functionResult = scriptFunction();
	if (!functionResult.valid())
	{
		sol::error error = functionResult;
		std::print("Script error: {}\n", error.what());

		return;
	}

	m_onStart = m_environment["OnStart"];
	m_onUpdate = m_environment["OnUpdate"];
}

ScriptInstance::~ScriptInstance()
{
}

void ScriptInstance::OnStart()
{
	if (m_onStart.valid())
	{
		sol::protected_function_result result = m_onStart();

		if (!result.valid())
		{
			sol::error err = result;
			std::print("onStart error: {}\n", err.what());
			// TODO: Send to error logger when it exists
		}
	}
}

void ScriptInstance::OnUpdate(float deltatime)
{
	if (m_onUpdate.valid())
	{
		sol::protected_function_result result = m_onUpdate(deltatime);

		if (!result.valid())
		{
			sol::error err = result;
			std::print("ScriptInstance: Error in onUpdate for script {}: {}\n", m_scriptPath, err.what());
			// TODO: Send to error logger when it exists
		}
	}
}

bool ScriptInstance::Reload(sol::load_result &p_script)
{
	if (!p_script.valid())
	{
		return false;
	}

	sol::protected_function scriptFunction = p_script;

	sol::set_environment(m_environment, scriptFunction);

	sol::protected_function_result functionResult = scriptFunction();
	if (!functionResult.valid())
	{
		sol::error error = functionResult;
		std::print("Script error: {}\n", error.what());

		return false;
	}

	m_onStart = m_environment["onStart"];
	m_onUpdate = m_environment["onUpdate"];

	return true;
}

std::string ScriptInstance::GetScriptPath()
{
	return m_scriptPath;
}
