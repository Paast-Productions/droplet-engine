#include "ScriptInstance.hpp"
#include "Node.hpp"
#include <print>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet;
using namespace Droplet::Script;

ScriptInstance::ScriptInstance(Scene::Node *p_owner, GameInput &p_input, LuaStateHandler &p_stateHandler, sol::load_result &p_script, const std::string &p_scriptPath) :
	m_owner(p_owner),
	m_stateHandler(p_stateHandler),
	m_environment(m_stateHandler.GetState(), sol::create, m_stateHandler.GetState().globals()),
	m_scriptPath(p_scriptPath)
{
	ZoneScoped;

	if (m_owner) 
	{
		m_environment["self"] = p_owner;
	}

	sol::protected_function scriptFunction = p_script;

	sol::set_environment(m_environment, scriptFunction);

	sol::protected_function_result functionResult = scriptFunction();
	if (!functionResult.valid())
	{
		sol::error error = functionResult;
		std::print("Script error: {}\n", error.what());

		return;
	}

	m_environment["Input"] = &p_input;
	m_onStart = m_environment["OnStart"];
	m_onUpdate = m_environment["OnUpdate"];
}

ScriptInstance::~ScriptInstance()
{
	ZoneScoped;

}

void ScriptInstance::OnStart()
{
	ZoneScoped;

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
	ZoneScoped;

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
	ZoneScoped;

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

	m_onStart = m_environment["OnStart"];
	m_onUpdate = m_environment["OnUpdate"];

	return true;
}

void ScriptInstance::RenderInternalUI() 
{
	ZoneScoped;

	sol::protected_function renderUI = m_environment["RenderUI"];

	if (!renderUI.valid())
	{
		return;
	}

	sol::protected_function_result result = renderUI();

	if (!result.valid())
	{
		sol::error error = result;
	}
}

std::string ScriptInstance::GetScriptPath()
{
	return m_scriptPath;
}
