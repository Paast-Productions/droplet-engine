
#include "LadderBehaviour.hpp"

#include <SceneSystem/Scene.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Components/LadderComponent.hpp>
#include <SceneSystem/Node.hpp>

#include <algorithm>

using namespace Droplet::Scene;

void LadderBehaviour::StartImpl()
{
}

void LadderBehaviour::AddLadderComponent(
    std::weak_ptr<LadderComponent> p_ladderComponent)
{
    auto ladder = p_ladderComponent.lock();

    if (!ladder)
    {
        return;
    }

    // Avoid registering the same component twice.
    for (const auto &existing : m_ladderComponents)
    {
        if (auto existingShared = existing.lock())
        {
            if (existingShared == ladder)
            {
                return;
            }
        }
    }

    m_ladderComponents.push_back(p_ladderComponent);
}

void LadderBehaviour::RemoveLadderComponent(
    LadderComponent *p_ladderComponent)
{
    if (!p_ladderComponent)
    {
        return;
    }

    std::erase_if(
        m_ladderComponents,
        [p_ladderComponent](const std::weak_ptr<LadderComponent> &p_existing)
        {
            auto existing = p_existing.lock();

            return !existing || existing.get() == p_ladderComponent;
        }
    );
}


void LadderBehaviour::LoadTargetScene(const std::string &p_targetSceneName)
{
    auto sceneManager = GetSceneManager();

    if (!sceneManager)
    {
        return;
    }

	if (sceneManager->GetScene(p_targetSceneName))
    {
        return;

	}
    sceneManager->LoadFromFile(ROOT_PATH "/GameData/Scenes/" + p_targetSceneName + ".json");
}

void LadderBehaviour::BeginTransition(
    const std::string &p_targetSceneName,
    std::weak_ptr<Scene> p_currentScene)
{
    auto currentScene = p_currentScene.lock();

    if (!currentScene)
    {
        return;
    }

    auto sceneManager = GetSceneManager();

    if (!sceneManager)
    {
        return;
    }

    auto targetScene = sceneManager->GetScene(p_targetSceneName);

    if (!targetScene || targetScene == currentScene)
    {
        return;
    }

    // Switch active scenes.
    sceneManager->ActivateScene(targetScene->GetName());
    sceneManager->DeactivateScene(currentScene->GetName());
}
