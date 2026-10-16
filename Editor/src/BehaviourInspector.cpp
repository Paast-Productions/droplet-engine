#include "BehaviourInspector.hpp"
#include <Engine.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <algorithm>
#include <string>
#include <vector>

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
    const float width = ImGui::GetContentRegionAvail().x;
    Droplet::Scene::Behaviour *behaviourToOpen = nullptr;

    if (ImGui::Button("+ Add Behaviour", ImVec2(width, 0.0f)))
    {
        ImGui::OpenPopup("Add Behaviour");
    }

    if (ImGui::BeginPopup("Add Behaviour"))
    {
        std::vector<std::string> behaviourTypes;
        for (const auto &[name, factory] : Droplet::Scene::BehaviourRegistry::GetRegistry())
        {
            (void)factory;
            behaviourTypes.push_back(name);
        }
        std::sort(behaviourTypes.begin(), behaviourTypes.end());

        if (behaviourTypes.empty())
        {
            ImGui::TextDisabled("No behaviours registered");
        }

        for (const std::string &name : behaviourTypes)
        {
            if (ImGui::MenuItem(name.c_str()))
            {
                behaviourToOpen = &m_editorContext->GetEngine().AddBehaviour(name);
            }

        }
        ImGui::EndPopup();
    }

    ImGui::Separator();

    Droplet::Scene::Behaviour *behaviourToRemove = nullptr;
    for (const auto &behaviour : m_editorContext->GetEngine().GetBehaviours())
    {
        if (!behaviour)
        {
            continue;
        }


        ImGui::PushID(behaviour.get());
        const std::string typeName(behaviour->GetTypeName());

        if (behaviour.get() == behaviourToOpen)
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }

        const bool isOpen = ImGui::CollapsingHeader(typeName.c_str());

        if (ImGui::BeginPopupContextItem("Behaviour Options"))
        {
            if (ImGui::MenuItem("Remove Behaviour"))
            {
                behaviourToRemove = behaviour.get();
            }
            ImGui::EndPopup();
        }

        if (isOpen)
        {
            bool active = behaviour->IsActive();
            if (ImGui::Checkbox("Active", &active))
            {
                behaviour->SetActive(active);
            }

            behaviour->RenderUI();
        }

        ImGui::PopID();
    }

    // Erase after iteration so the vector and current behaviour stay valid
    // until this frame's UI rendering is complete.
    if (behaviourToRemove != nullptr)
    {
        m_editorContext->GetEngine().RemoveBehaviour(*behaviourToRemove);
    }
}

void BehaviourInspector::RenderToolbar()
{
}
