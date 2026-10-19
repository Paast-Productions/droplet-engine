#include "ScriptComponent.hpp"
#include "ImGui/imgui.h"

#include <print>

using namespace Droplet::Scene;
using namespace Droplet::Script;

void ScriptComponent::Start()
{
	ScriptSystem::Get().Call(this, "OnStart");
}

void ScriptComponent::Update([[maybe_unused]] float p_deltaTime)
{

}

const std::string &ScriptComponent::GetScriptPath() const
{
    return m_scriptPath;
}

void Droplet::Scene::ScriptComponent::AttachScript(const std::string p_scriptPath)
{
    if (m_scriptPath == p_scriptPath)
    {
        return;
    }
    
    if (m_started && p_scriptPath.empty())
    {
        ScriptSystem::Get().DetachComponentScript(this);
    }

    m_scriptPath = p_scriptPath;

    if (m_scriptPath.empty())
    {
        return;
    }

    ScriptSystem::Get().CreateComponentScript(this, m_scriptPath);
    ScriptSystem::Get().ActivateComponentScript(this);
    ScriptSystem::Get().Start();
}

void ScriptComponent::DetachScript()
{
    if (m_scriptPath.empty())
    {
        return;
    }

    ScriptSystem::Get().DetachComponentScript(this);
}

void ScriptComponent::ActivateScript()
{
    ScriptSystem::Get().ActivateComponentScript(this);
}

void ScriptComponent::DeactivateScript()
{
    ScriptSystem::Get().DeactivateComponentScript(this);
}

void ScriptComponent::RenderUIImpl()
{
    ImGui::Text("Assigned: %s", m_scriptPath.empty() ? "None" : m_scriptPath.c_str(), ImVec2(180.0f, 0.0f));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT_PATH"))
        {
            if (payload->Data != nullptr && payload->DataSize > 0)
            {
                const char* droppedPath = static_cast<const char*>(payload->Data);
                AttachScript(droppedPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::BeginDisabled(m_scriptPath.empty());

    if (ImGui::Button("Detach Script"))
    {
        AttachScript("");
    }

    ImGui::EndDisabled();

	ImGui::BeginChild("ScriptComponentChild", ImVec2(0.0f, 0.0f), true | ImGuiChildFlags_AutoResizeY );

    ScriptSystem::Get().Call(this, "RenderUI");

    ImGui::EndChild();
}

nlohmann::json ScriptComponent::SerializeImpl()
{
    nlohmann::json json;

    json["filepath"] = GetScriptPath();

    // TODO

    return json;
}

void ScriptComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json  p_compJson)
{
	// TODO: Implement deserialization logic for the script component.
}
