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
	const std::string preview = m_scriptPath.empty()
		? "Drop Lua script here"
		: m_scriptPath;

	ImGui::Button(preview.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));
	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("SCRIPT_PATH"))
		{
			if (payload->Data != nullptr && payload->DataSize > 0)
			{
				std::string scriptPath(static_cast<const char *>(payload->Data),
					static_cast<std::size_t>(payload->DataSize));
				if (!scriptPath.empty() && scriptPath.back() == '\0')
				{
					scriptPath.pop_back();
				}
				AttachScript(scriptPath);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (!m_scriptPath.empty() && ImGui::Button("Detach Script"))
	{
		AttachScript("");
	}

	ImGui::Separator();
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
