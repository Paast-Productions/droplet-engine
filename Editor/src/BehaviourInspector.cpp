#include "BehaviourInspector.hpp"
#include <Engine.hpp>

using namespace Droplet::Editor;

BehaviourInspector::BehaviourInspector(std::shared_ptr<Droplet::Engine> p_engine) : EditorWindow(p_engine)
{
}

void BehaviourInspector::InitImpl()
{
	SetName("Behaviours");
}

void BehaviourInspector::CloseImpl()
{
}

void BehaviourInspector::RenderImpl()
{
    ImGui::Button("Drop Lua behaviour here", ImVec2(180.0f, 0.0f));

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("SCRIPT_PATH"))
        {
            if (payload->Data != nullptr && payload->DataSize > 0)
            {
                const std::string scriptPath(
                    static_cast<const char *>(payload->Data));

                GetEngine()->AddScriptBehaviour(scriptPath);
            }
        }

        ImGui::EndDragDropTarget();
    }

    for (const auto &behaviour : GetEngine()->GetScriptBehaviours())
    {
        ImGui::TextUnformatted(behaviour->GetScriptPath().c_str());
    }
}

void BehaviourInspector::RenderToolbar()
{
}
