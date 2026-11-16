
#include "LadderComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>
#include <SceneSystem/Behaviours/LadderBehaviour.hpp>
#include <Editor/NodeSlot.hpp>

using namespace Droplet::Scene;

LadderComponent::~LadderComponent()
{
    if (auto ladderBehaviour = m_ladderBehaviour.lock())
    {
        ladderBehaviour->RemoveLadderComponent(this);
    }
}

void LadderComponent::ConstructImpl()
{
    auto scene = GetOwner()->GetScene();
    if (!scene)    
    {
        return;
    }

    // Retrieve the existing persistent behaviour or create it.
    auto ladderBehaviour = scene->GetBehaviour<LadderBehaviour>();

    if (!ladderBehaviour)
    {
        ladderBehaviour = std::dynamic_pointer_cast<LadderBehaviour>(
            scene->StartBehaviour("LadderBehaviour").lock()
        );
    }

    if (!ladderBehaviour)
    {
        return;
    }

    m_ladderBehaviour = ladderBehaviour;

    auto sharedThis = std::dynamic_pointer_cast<LadderComponent>(
        shared_from_this()
    );

    ladderBehaviour->AddLadderComponent(sharedThis);
}

void LadderComponent::UpdateImpl()
{
}

void LadderComponent::RenderUIImpl()
{
#ifndef _GAME

    float buttonWidth = ImGui::GetContentRegionAvail().x;

    ImGui::InputText("Target Scene Name", &m_targetSceneName);

    if (ImGui::Button("Transition"))
    {
        if (auto ladderBehaviour = m_ladderBehaviour.lock())
        {
            ladderBehaviour->BeginTransition(
                m_targetSceneName,
                GetOwner()->GetScene()
            );
        }
    }

#endif
}

nlohmann::json LadderComponent::SerializeImpl()
{
    nlohmann::json json;

    json["targetSceneName"] = m_targetSceneName;

    return json;
}

void LadderComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (p_compJson.contains("targetSceneName"))
    {
        m_targetSceneName = p_compJson["targetSceneName"];
    }
}

void LadderComponent::BeginTransition()
{
    if (auto ladderBehaviour = m_ladderBehaviour.lock())
    {
        ladderBehaviour->BeginTransition(
            m_targetSceneName,
            GetOwner()->GetScene()
        );  
    }
}

