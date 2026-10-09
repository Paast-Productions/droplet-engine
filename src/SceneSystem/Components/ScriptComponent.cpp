#include "ScriptComponent.hpp"
#include "ImGui/imgui.h"

#include <print>

using namespace Droplet::Scene;
using namespace Droplet::Script;

ScriptComponent::ScriptComponent(const std::string &p_scriptPath)
    : m_scriptPath(p_scriptPath), m_started(false)
{

}

void ScriptComponent::Start()
{
    m_started = true;

    //TODO: Should probably be moved somewhere more efficient, constructior or separate lode function,
    auto &scriptSystem = ScriptSystem::Get();

    scriptSystem.CreateComponentScript(this, m_scriptPath);
    scriptSystem.ActivateComponentScript(this);
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
    
    if (m_started)
    {
        ScriptSystem::Get().DetachComponentScript(this);
    }

    m_scriptPath = p_scriptPath;

    if (m_started && m_scriptPath.empty())
    {
        ScriptSystem::Get().CreateComponentScript(this, m_scriptPath);
        ScriptSystem::Get().ActivateComponentScript(this);
    }
}

void ScriptComponent::DetachScript()
{
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
    ImGui::TextUnformatted("Script-Component");
    ImGui::Separator();

    ImGui::Text("Script: %s", m_scriptPath.c_str());

    if (!m_scriptPath.empty())
    {
        ImGui::Text("Script: %s", m_scriptPath.c_str());
    }
    else 
    {
        ImGui::Text("Script: None");
    }

    if (ImGui::Button("Attach Script"))
    {
        ImGui::OpenPopup("AttachScriptPopup");
    }

    if (ImGui::BeginPopup("attachScriptPopup"))
    {
        static const std::string scripts[] =
        {
            "testScript.lua",
            "testScript2.lua"
        };

        for (const std::string& script : scripts)
        {
            const bool selected = m_scriptPath == script;

            if (ImGui::Selectable(script.c_str(), selected))
            {
                AttachScript(script);
                ImGui::CloseCurrentPopup();
            }

            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine();

    ImGui::BeginDisabled(m_scriptPath.empty());

    if (ImGui::Button("Detach Script"))
    {
       // Detach Script
    }

    ImGui::EndDisabled();
}

nlohmann::json ScriptComponent::SerializeImpl()
{
    nlohmann::json json;

    json["type"] = "ScriptComponent";
    json["filepath"] = GetScriptPath();

    // TODO

    return json;
}

void ScriptComponent::DeserializeImpl([[maybe_unused]] nlohmann::json  p_compJson)
{
    // TODO
}
