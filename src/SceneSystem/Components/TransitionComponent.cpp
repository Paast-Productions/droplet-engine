
#include "TransitionComponent.hpp"

#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>
#include <SceneSystem/Behaviours/TransitionBehaviour.hpp>
#include <Editor/NodeSlot.hpp>

using namespace Droplet::Scene;

TransitionComponent::~TransitionComponent()
{
    if (auto transitionBehaviour = m_transitionBehaviour.lock())
    {
        transitionBehaviour->RemoveTransitionComponent(this);
    }
}

void TransitionComponent::ConstructImpl()
{
    auto scene = GetOwner()->GetScene();
    if (!scene)
    {
        return;
    }

    // Retrieve the existing persistent behaviour or create it.
    auto transitionBehaviour = scene->GetBehaviour<TransitionBehaviour>();

    if (!transitionBehaviour)
    {
        transitionBehaviour = std::dynamic_pointer_cast<TransitionBehaviour>(
            scene->StartBehaviour("TransitionBehaviour").lock()
        );
    }

    if (!transitionBehaviour)
    {
        return;
    }

    m_transitionBehaviour = transitionBehaviour;

    auto sharedThis = std::dynamic_pointer_cast<TransitionComponent>(
        shared_from_this()
    );

    transitionBehaviour->AddTransitionComponent(sharedThis);
}

void TransitionComponent::UpdateImpl()
{
    if (!m_targetSceneLoaded)
    {
        if (auto transitionBehaviour = m_transitionBehaviour.lock())
        {
			if (m_targetSceneName.empty())
			{
				return;
			}
            transitionBehaviour->LoadTargetScene(m_targetSceneName);
			m_targetSceneLoaded = true;
        }
    }
}

void TransitionComponent::RenderUIImpl()
{
#ifndef _GAME

    float buttonWidth = ImGui::GetContentRegionAvail().x;

    ImGui::InputText("Target Scene Name", &m_targetSceneName);

    if (ImGui::Button("Transition"))
    {
        if (auto transitionBehaviour = m_transitionBehaviour.lock())
        {
            transitionBehaviour->BeginTransition(
                m_targetSceneName,
                GetOwner()->GetScene()
            );
        }
    }

#endif
}

nlohmann::json Droplet::Scene::TransitionComponent::SerializeImpl()
{
    nlohmann::json json;

    json["targetSceneName"] = m_targetSceneName;

    return json;
}

void Droplet::Scene::TransitionComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
    if (p_compJson.contains("targetSceneName"))
    {
        m_targetSceneName = p_compJson["targetSceneName"];
    }
}

void TransitionComponent::BeginTransition()
{
    if (auto transitionBehaviour = m_transitionBehaviour.lock())
    {
        transitionBehaviour->BeginTransition(
            m_targetSceneName,
            GetOwner()->GetScene()
        );  
    }
}
