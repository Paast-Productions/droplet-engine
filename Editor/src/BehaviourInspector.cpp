#include "BehaviourInspector.hpp"
#include <Engine.hpp>

using namespace Droplet::Editor;

BehaviourInspector::BehaviourInspector(EditorContext *editorContext) 
    : m_editorContext(editorContext){}

void BehaviourInspector::InitImpl()
{
	SetName("Behaviours");
}

void BehaviourInspector::CloseImpl()
{
}

void BehaviourInspector::RenderImpl()
{
    const ImVec2 dropAreaSize(ImGui::GetContentRegionAvail().x, 50.0f);

    ImGui::Button("Drop Lua behaviour here", dropAreaSize);

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("SCRIPT_PATH"))
        {
            if (payload->Data != nullptr && payload->DataSize > 0)
            {
                const std::string scriptPath(
                    static_cast<const char *>(payload->Data));

                m_editorContext->GetEngine().AddScriptBehaviour(scriptPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    for (const auto &behaviour : m_editorContext->GetEngine().GetScriptBehaviours())
    {
        const std::string scriptName = behaviour->GetScriptPath();

        ImGui::PushID(behaviour.get());

        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

        ImGui::Button(scriptName.c_str(), 
            ImVec2(ImGui::GetContentRegionAvail().x, 30.0f)
        );

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
			ImGui::OpenPopup("Behaviour Options");
        }

		ImGui::PopStyleColor(3);

        if (ImGui::BeginPopup("Behaviour Options"))
        {
            if (ImGui::MenuItem("Detach Behaviour"))
            {
                m_editorContext->GetEngine().RemoveScriptBehaviour(*behaviour);
            }
            ImGui::EndPopup();
        }

        ImGui::PopID();
    }
}

void BehaviourInspector::RenderToolbar()
{
}
