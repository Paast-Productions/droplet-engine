#include "BehaviourInspector.hpp"
#include <Engine.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>
#include <algorithm>
#include <string>
#include <vector>

using namespace Droplet::Editor;

void BehaviourInspector::InitImpl()
{
	SetName("Behaviours");
}

void BehaviourInspector::RenderImpl()
{
    auto &sceneManager = m_editorContext->GetEngine().GetSceneManager();

    const float width = ImGui::GetContentRegionAvail().x;
    Droplet::Scene::Behaviour *behaviourToOpen = nullptr;

    if (ImGui::Button("+ Add Behaviour", ImVec2(width * 0.66f, 0.0f)))
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
                if (auto behaviour = sceneManager.StartBehaviour(name))
                {
                    behaviourToOpen = behaviour;
                }
            }

        }
        ImGui::EndPopup();
    }
    
    ImGui::SameLine();
    if(ImGui::Button("Save Behaviours", ImVec2(width / 3.0f, 0.0f)))
    {
        sceneManager.SerializeBehaviours();
    }

    ImGui::Separator();


    Droplet::Scene::Behaviour *behaviourToRemove = nullptr;
    bool removeRequest = false;

    for (const auto &behaviour : sceneManager.GetBehaviours())
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
                removeRequest = true;
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

    if (removeRequest)
    {
        sceneManager.ShutdownBehaviour(behaviourToRemove);
    }
}

void BehaviourInspector::RenderToolbar()
{
}
