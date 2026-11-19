#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
#include <SceneSystem/Behaviour.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Items/BaseItems.hpp>


namespace Droplet::Scene
{
    void PlayerComponent::StartImpl()
    {
        auto owner = GetOwner();
        if (!owner)
            return;

        auto scene = owner->GetScene();
        if (!scene)
            return;

        auto playerBehaviour = scene->GetBehaviour<PlayerBehaviour>();

        if (!playerBehaviour)
        {
            auto startedBehaviour = scene->StartBehaviour("PlayerBehaviour").lock();

            playerBehaviour = std::dynamic_pointer_cast<PlayerBehaviour>(startedBehaviour);
        }

        if (!playerBehaviour)
            return;

        m_playerBehaviour = playerBehaviour;
        playerBehaviour->SetPlayer(owner);
    }

    void PlayerComponent::UpdateImpl()
    {
        // No per-frame logic is currently required.
    }


    void PlayerComponent::RenderUIImpl()
    {
        const char *items[] = { "HeatLevel", "FreezeLevel", "PressureLevel" };
        static int item_current = 0;
        ImGui::Combo("combo", &item_current, items, IM_COUNTOF(items));
        for(int x = 1; x <= 5; x++)
        {
            std::string lebel = "Level" + std::to_string(x);
            ImGui::RadioButton(lebel.c_str(), &m_level, x);
            ImGui::SameLine();
        }

        switch (item_current)
        {
            case HeatLevel:
            {
                m_playerBehaviour.lock()->SetHeatLevel(m_level);
                break;
            }
            case FreezeLevel:
            {
                m_playerBehaviour.lock()->SetFreezeLevel(m_level);
                break;
            }
            case PressureLevel:
            {
                m_playerBehaviour.lock()->SetPressureLevel(m_level);
                break;
            }
        }

        ImGui::SameLine();
    }

    nlohmann::json PlayerComponent::SerializeImpl()
    {
        return nlohmann::json::object();
    }

    void PlayerComponent::DeserializeImpl(const nlohmann::json p_compJson)
    {
        StartImpl();
    }
}