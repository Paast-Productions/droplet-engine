
#include "TransitionBehaviour.hpp"

#include <SceneSystem/Scene.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Components/TransitionComponent.hpp>
#include <SceneSystem/Node.hpp>

#include <algorithm>

using namespace Droplet::Scene;

void Droplet::Scene::TransitionBehaviour::AddTransitionComponent(std::weak_ptr<TransitionComponent> p_transitionComponent)
{
	auto transition = p_transitionComponent.lock();

	if (!transition)
	{
		return;
	}

	// Do not register same component twice
	for (const auto &existing : m_transitionComponents)
	{
		if (auto existingShared = existing.lock())
		{
			if (existingShared == transition)
			{
				return;
			}
		}
	}

	m_transitionComponents.push_back(p_transitionComponent);
}

void Droplet::Scene::TransitionBehaviour::RemoveTransitionComponent(TransitionComponent *p_transitionComponent)
{
	if (!p_transitionComponent)
	{
		return;
	}

	std::erase_if(
		m_transitionComponents,
		[p_transitionComponent](const std::weak_ptr<TransitionComponent> &p_existing)
		{
			auto existing = p_existing.lock();

			return !existing || existing.get() == p_transitionComponent;
		}
	);
}

void Droplet::Scene::TransitionBehaviour::BeginTransition(const std::string &p_targetSceneName, std::weak_ptr<Scene> p_currentScene)
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

void Droplet::Scene::TransitionBehaviour::LoadTargetScene(const std::string &p_targetSceneName)
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

void TransitionBehaviour::StartImpl()
{
}
