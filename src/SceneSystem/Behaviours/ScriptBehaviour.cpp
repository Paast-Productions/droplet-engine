#include "ScriptBehaviour.hpp"
#include "ImGui/imgui.h"
#include <print>

using namespace Droplet::Scene;

const std::string &ScriptBehaviour::GetScriptPath() const
{
	return m_scriptPath;
}

void ScriptBehaviour::SetScriptPath(const std::string &p_scriptPath)
{
	m_scriptPath = p_scriptPath;
}

void ScriptBehaviour::DetachScript()
{
	ScriptSystem::Get().DetachBehaviourScript(this);
}

void ScriptBehaviour::ActivateScript()
{
	ScriptSystem::Get().ActivateBehaviourScript(this);
}

void ScriptBehaviour::DeactivateScript()
{
	ScriptSystem::Get().DeactivateBehaviourScript(this);
}

void ScriptBehaviour::RenderUIImpl()
{

}

void ScriptBehaviour::AttachScript(const std::string p_scriptPath)
{
    if (m_scriptPath == p_scriptPath)
    {
        return;
    }

    if (p_scriptPath.empty())
    {
        ScriptSystem::Get().DetachBehaviourScript(this);
    }

    m_scriptPath = p_scriptPath;

    if (m_scriptPath.empty())
    {
        return;
    }

	ScriptSystem::Get().CreateBehaviourScript(this, m_scriptPath);
	ScriptSystem::Get().ActivateBehaviourScript(this);
}
