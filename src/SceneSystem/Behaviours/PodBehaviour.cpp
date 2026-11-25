#include "PodBehaviour.hpp"
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/SceneManager.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Components/PodComponent.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <Time.hpp>
#include <iostream>

using namespace Droplet::Scene;

void PodBehaviour::StartImpl()
{
	LoadPodScene();
}

void PodBehaviour::PreUpdateImpl()
{
	LoadPodScene();

	if (!m_pendingUnloadScene.expired())
	{
		std::string name = m_pendingUnloadScene.lock()->GetName();
		GetSceneManager()->UnloadScene(name);
		if ((name == "BoatScene") && (GetSceneManager()->GetScene("InsideBoatScene")))
		{	
			GetSceneManager()->UnloadScene("InsideBoatScene");
		}
		m_pendingUnloadScene = {};
	}

	UpdatePendingTransition();

	if (m_transitionTimer > 0.0f)
	{
		m_transitionTimer -= Time::Get().GetDeltaTime();

		if (m_transitionTimer <= 0.0f)
		{
			// Transition complete, activate the target scene and deactivate the current scene

			// If moving to the boat scene, set the boat player node's position and rotation
			// to match the pod player node's position and rotation relative to the pod node.
			if (auto targetScene = m_targetScene.lock())
			{
				if (targetScene->GetName() == "BoatScene")
				{
					auto boatPod = GetPodInScene(targetScene).lock();
					auto podPod = GetPodInScene(m_podScene).lock();

					auto boatPlayerNode = boatPod->GetPlayerNode().lock();
					auto podPlayerNode = podPod->GetPlayerNode().lock();

					std::shared_ptr<Node> boatPlayerHeadNode = nullptr;
					for (const auto &child : boatPlayerNode->GetChildren())
					{
						if (child->GetComponent<CameraComponent>())
						{
							boatPlayerHeadNode = child;
							break;
						}
					}

					std::shared_ptr<Node> podPlayerHeadNode = nullptr;
					for (const auto &child : podPlayerNode->GetChildren())
					{
						if (child->GetComponent<CameraComponent>())
						{
							podPlayerHeadNode = child;
							break;
						}
					}

					// Ensure all nodes are valid before proceeding
					if (boatPlayerNode && podPlayerNode && boatPlayerHeadNode && podPlayerHeadNode)
					{
						// Set the boat player node's position and rotation to match the pod 
						// player node's position and rotation relative to the pod node.

						boatPlayerNode->GetTransform().SetMatrix(
							podPlayerNode->GetTransform().GetMatrix(Transform::Space::World),
							Transform::Space::World
						);

						boatPlayerHeadNode->GetTransform().SetMatrix(
							podPlayerHeadNode->GetTransform().GetMatrix(Transform::Space::Local),
							Transform::Space::Local
						);
					}
				}

				// Activate the target scene
				GetSceneManager()->ActivateScene(targetScene->GetName());

				// Call the target pod's OnTransitionToThisPod() method to handle any additional logic for the transition.
				auto podInTargetScene = GetPodInScene(targetScene).lock();
				if (podInTargetScene)
				{
					podInTargetScene->OnTransitionToThisPod();
				}

				// Deactivate the current scene (the pod scene)
				if (auto podScene = m_podScene.lock())
				{
					GetSceneManager()->DeactivateScene(podScene->GetName());
				}
			}

			// Clear the target scene and reset the transition timer
			m_transitionTimer = 0.0f;
			m_targetScene = {};
		}
	}
}

void PodBehaviour::RenderUIImpl()
{
#ifdef _EDITOR

	ImGui::DragFloat("Transition Time", &m_transitionTime, 0.05f, 1.0f);

	// Show all registered PodComponents along with the containing scene in a table.

	if (ImGui::BeginTable("PodComponentsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Scene");
		ImGui::TableSetupColumn("Node");

		for (const auto &podComponent : m_podComponents)
		{
			auto podComponentShared = podComponent.lock();
			if (!podComponentShared)
			{
				continue; // Skip expired weak pointers
			}

			auto scene = podComponentShared->GetOwner()->GetScene();
			if (!scene)
			{
				continue; // Skip if the scene is not valid
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(scene->GetName().c_str());

			ImGui::TableSetColumnIndex(1);
			ImGui::Text(podComponentShared->GetOwner()->GetName().c_str());
		}

		ImGui::EndTable();
	}

#endif
}


void PodBehaviour::LoadPodScene()
{
	// Load the pod scene if not already loaded. Scenes queued at startup are not tracked
	// by m_loadingScenes, so wait for them to finish instead of requesting a duplicate.
	if (m_podScene.expired())
	{
		m_podScene = GetSceneManager()->GetScene("PodScene");

		if (m_podScene.expired() && !GetSceneManager()->HasPendingAsyncLoads())
		{
			RequestSceneLoad("PodScene");
		}
	}
}

void PodBehaviour::RequestSceneLoad(const std::string &p_sceneName)
{
	if (GetSceneManager()->GetScene(p_sceneName) || !m_loadingScenes->insert(p_sceneName).second)
	{
		return; // Already loaded or already loading
	}

	// The callback runs on the main thread after the scene is registered, so it only touches the shared set
	auto loadingScenes = m_loadingScenes;
	GetSceneManager()->LoadFromFileAsync(
		ROOT_PATH "/GameData/Scenes/" + p_sceneName + ".json",
		[loadingScenes, p_sceneName](std::shared_ptr<Scene>) { loadingScenes->erase(p_sceneName); });
}

void PodBehaviour::AddPodComponent(std::weak_ptr<PodComponent> p_podComponent)
{
	if (p_podComponent.expired())
	{
		return; // Invalid PodComponent
	}

	for (const auto &existingPodComponent : m_podComponents)
	{
		auto existingPodComponentShared = existingPodComponent.lock();
		if (!existingPodComponentShared)
		{
			continue; // Skip expired weak pointers
		}

		if (existingPodComponentShared == p_podComponent.lock())
		{
			return; // Already added
		}

		if (existingPodComponentShared->GetOwner()->GetScene() == p_podComponent.lock()->GetOwner()->GetScene())
		{
			return; // A PodComponent from the same scene is already registered
		}
	}

	m_podComponents.push_back(p_podComponent);
}

void PodBehaviour::RemovePodComponent(PodComponent *p_podComponent)
{
	if (!p_podComponent)
	{
		return; // Invalid PodComponent
	}

	auto it = std::remove_if(m_podComponents.begin(), m_podComponents.end(),
		[&p_podComponent](const std::weak_ptr<PodComponent> &existingPodComponent) {
			auto existingPodComponentShared = existingPodComponent.lock();
			if (!existingPodComponentShared)
			{
				return true; // Remove expired weak pointers
			}
			return existingPodComponentShared.get() == p_podComponent;
		});

	m_podComponents.erase(it, m_podComponents.end());
}

std::weak_ptr<PodComponent> PodBehaviour::GetPodInScene(std::weak_ptr<Scene> p_scene) const
{
	auto sceneShared = p_scene.lock();
	if (!sceneShared)
	{
		return std::weak_ptr<PodComponent>();
	}

	for (const auto &podComponent : m_podComponents)
	{
		auto podComponentShared = podComponent.lock();
		if (!podComponentShared)
		{
			continue;
		}

		if (podComponentShared->GetOwner()->GetScene() == sceneShared)
		{
			return podComponent;
		}
	}

	return std::weak_ptr<PodComponent>();
}

std::weak_ptr<PodComponent> PodBehaviour::GetPodInScene(const std::string &p_sceneName) const
{
	for (const auto &podComponent : m_podComponents)
	{
		auto podComponentShared = podComponent.lock();
		if (!podComponentShared)
		{
			continue;
		}

		if (podComponentShared->GetOwner()->GetScene()->GetName() == p_sceneName)
		{
			return podComponent;
		}
	}

	return std::weak_ptr<PodComponent>();
}

void PodBehaviour::BeginTransition(const std::string &p_targetSceneName, std::weak_ptr<Scene> p_currentScene)
{
	if (m_transitionTimer > 0.0f || m_pendingTransition)
	{
		return; // Already performing a transition
	}

	if (p_currentScene.expired())
	{
		return; // Invalid scenes
	}

	auto &sceneManager = *GetSceneManager();

	// Start loading the target now. The transition timer starts once the target is ready.
	if (p_targetSceneName == "BoatScene" && !sceneManager.GetScene(p_targetSceneName) && m_loadingScenes->count(p_targetSceneName) == 0)
	{
		m_transferInventoryOnBoatLoad = true;
	}
	RequestSceneLoad(p_targetSceneName);
	if (p_targetSceneName == "BoatScene")
	{
		RequestSceneLoad("InsideBoatScene");
	}

	// If moving from the boat scene, set the pod player node's position and rotation 
	// to match the boat player node's position and rotation relative to the pod node.
	if (auto currentSceneShared = p_currentScene.lock())
	{
		if (currentSceneShared->GetName() == "BoatScene")
		{
			auto boatPod = GetPodInScene(currentSceneShared).lock();
			auto podPod = GetPodInScene(m_podScene).lock();

			auto boatPlayerNode = boatPod->GetPlayerNode().lock();
			auto podPlayerNode = podPod->GetPlayerNode().lock();

			std::shared_ptr<Node> boatPlayerHeadNode = nullptr;
			for (const auto &child : boatPlayerNode->GetChildren())
			{
				if (child->GetComponent<CameraComponent>())
				{
					boatPlayerHeadNode = child;
					break;
				}
			}

			std::shared_ptr<Node> podPlayerHeadNode = nullptr;
			for (const auto &child : podPlayerNode->GetChildren())
			{
				if (child->GetComponent<CameraComponent>())
				{
					podPlayerHeadNode = child;
					break;
				}
			}

			// Ensure all nodes are valid before proceeding
			if (boatPlayerNode && podPlayerNode && boatPlayerHeadNode && podPlayerHeadNode)
			{
				// Set the pod player node's position and rotation to match the boat 
				// player node's position and rotation relative to the pod node.

				podPlayerNode->GetTransform().SetMatrix(
					boatPlayerNode->GetTransform().GetMatrix(Transform::Space::World),
					Transform::Space::World
				);

				podPlayerHeadNode->GetTransform().SetMatrix(
					boatPlayerHeadNode->GetTransform().GetMatrix(Transform::Space::Local),
					Transform::Space::Local
				);
			}
		}
	}

	// Set the current scene to inactive and activate the pod scene
	if (auto currentSceneShared = p_currentScene.lock())
	{
		sceneManager.DeactivateScene(currentSceneShared->GetName());
		m_pendingUnloadScene = currentSceneShared;
	}

	if (auto podSceneShared = m_podScene.lock())
	{
		sceneManager.ActivateScene(podSceneShared->GetName());
	}

	m_pendingTransition = PendingTransition{ p_targetSceneName, p_currentScene };
	UpdatePendingTransition();
}

void PodBehaviour::UpdatePendingTransition()
{
	if (!m_pendingTransition || m_transitionTimer > 0.0f)
	{
		return;
	}

	auto &sceneManager = *GetSceneManager();
	const std::string targetName = m_pendingTransition->targetSceneName;

	auto targetScene = sceneManager.GetScene(targetName);
	if (!targetScene)
	{
		if (m_loadingScenes->count(targetName) == 0)
		{
			// The load finished without registering the scene, so it will never become ready
			std::cerr << "Pod transition to '" << targetName << "' cancelled: scene failed to load.\n";
			m_pendingTransition.reset();
		}
		return;
	}

	if (targetName == "BoatScene" && !sceneManager.GetScene("InsideBoatScene"))
	{
		return; // Wait for the inside scene, which is loaded alongside the boat scene
	}

	if (GetPodInScene(targetName).expired())
	{
		if (m_loadingScenes->count(targetName) == 0)
		{
			std::cerr << "Pod transition to '" << targetName << "' cancelled: no pod in scene.\n";
			m_pendingTransition.reset();
		}
		return;
	}

	if (!targetScene->AreResourcesLoaded())
	{
		return; // Wait for the target's resources so they do not pop in after the transition
	}

	if (m_transferInventoryOnBoatLoad && targetName == "BoatScene")
	{
		m_transferInventoryOnBoatLoad = false;
		if (auto podScene = m_podScene.lock())
		{
			podScene->GetBehaviour<PlayerBehaviour>()->TransferDroneInventoryToPlayer();
		}
	}

	StartTransition(targetScene);
}

void PodBehaviour::StartTransition(std::shared_ptr<Scene> p_targetScene)
{
	m_targetScene = p_targetScene;
	m_pendingTransition.reset();
	m_transitionTimer = m_transitionTime; // Start the transition timer
}
