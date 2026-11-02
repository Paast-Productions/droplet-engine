#include "ScriptComponent.hpp"
#include "ImGui/imgui.h"

#include <print>

#include <Time.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Script;

void ScriptComponent::StartImpl()
{

}

void ScriptComponent::UpdateImpl()
{
    ScriptSystem::Get().UpdateComponent(this, Droplet::Time::Get().GetDeltaTime());
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

    ScriptSystem::Get().StartComponent(this);
}

void ScriptComponent::DetachScript()
{
    if (m_scriptPath.empty())
    {
        return;
    }

    ScriptSystem::Get().DetachComponentScript(this);
}

void ScriptComponent::RenderUIImpl()
{
    const std::string preview = m_scriptPath.empty()
        ? "Drop Lua script here"
        : m_scriptPath;

    ImGui::Button(preview.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 32.0f));

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
    if (!p_compJson.contains("filepath"))
    {
        throw std::runtime_error("ScriptCompoent missing field 'filepath'");
    }
    AttachScript(p_compJson["filepath"]);
    ScriptSystem::Get().StartComponent(this);
}
