#include "ScriptSystem.hpp"
#include "LuaBindings.hpp"

#include <Engine/EngineFlags.hpp>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet;
using namespace Droplet::Script;

ScriptSystem::ScriptSystem(): m_luaStateHandler(), m_scriptManager(m_luaStateHandler)
{
	ZoneScoped;

	Bindings::LuaBindings::RegisterBindings(m_luaStateHandler.GetState());
}

ScriptSystem::~ScriptSystem()
{
	ZoneScoped;

}

void ScriptSystem::StartComponent(Droplet::Scene::Component *p_scriptComponent)
{
	ZoneScoped;

	m_scriptManager.StartComponent(p_scriptComponent);
}

void Droplet::Script::ScriptSystem::StartBehaviour(Droplet::Scene::Behaviour *p_scriptBehaviour)
{
	ZoneScoped;

	m_scriptManager.StartBehaviour(p_scriptBehaviour);
}

void ScriptSystem::UpdateComponent(Droplet::Scene::Component *p_scriptComponent, float p_deltaTime)
{
	ZoneScoped;

	m_scriptManager.UpdateComponent(p_scriptComponent, p_deltaTime);

	// Check for script file changes and reload if necessary, unless the SkipHotReloadScripts flag is set
	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipHotReloadScripts))
	{
		m_scriptManager.CheckForFileChanges();
	}
}

void Droplet::Script::ScriptSystem::UpdateBehaviour(Droplet::Scene::Behaviour *p_scriptBehaviour, float p_deltaTime)
{
	ZoneScoped;

	m_scriptManager.UpdateBehaviour(p_scriptBehaviour, p_deltaTime);

	// Check for script file changes and reload if necessary, unless the SkipHotReloadScripts flag is set
	if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipHotReloadScripts))
	{
		m_scriptManager.CheckForFileChanges();
	}
}

void ScriptSystem::CreateComponentScript(Scene::Component *p_sciptComponent, const std::string &p_scriptFile)
{
	ZoneScoped;
	ZoneText(p_scriptFile.c_str(), p_scriptFile.size());

	if (p_scriptFile.empty())
	{
		return;
	}

	m_scriptManager.CreateComponentScript(p_sciptComponent, p_scriptFile);
}

void ScriptSystem::CreateBehaviourScript(Scene::Behaviour *p_scriptBehaviour, const std::string &p_scriptFile)
{
	ZoneScoped;
	ZoneText(p_scriptFile.c_str(), p_scriptFile.size());

	if (p_scriptFile.empty())
	{
		return;
	}

	m_scriptManager.CreateBehaviourScript(p_scriptBehaviour, p_scriptFile);
}

void ScriptSystem::LoadScript(const std::string &p_scriptFile)
{
	ZoneScoped;
	ZoneText(p_scriptFile.c_str(), p_scriptFile.size());

	m_scriptManager.LoadScript(p_scriptFile);
}

void ScriptSystem::UnloadScript(const std::string &p_scriptFile)
{
	ZoneScoped;
	ZoneText(p_scriptFile.c_str(), p_scriptFile.size());

	m_scriptManager.UnloadScript(p_scriptFile);
}

void ScriptSystem::DetachComponentScript(Scene::Component *p_scriptComponent)
{
	ZoneScoped;

	m_scriptManager.DetachComponentScript(p_scriptComponent);
}

void ScriptSystem::DetachBehaviourScript(Scene::Behaviour *p_scriptBehaviour)
{
	ZoneScoped;

	m_scriptManager.DetachBehaviourScript(p_scriptBehaviour);
}

void ScriptSystem::SetScriptPath(const std::string &p_directoryPath)
{
	m_scriptManager.SetScriptDirectory(p_directoryPath);
}

std::string ScriptSystem::GetScriptPath()
{
	return m_scriptManager.GetScriptDirectory();
}

void ScriptSystem::DetachAllInstancesToScript(const std::string &p_scriptPath)
{
	ZoneScoped;

	m_scriptManager.DetachAllInstancesToScript(p_scriptPath);
}
