#include "PodBehaviour.hpp"
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Components/PodComponent.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <Time.hpp>

using namespace Droplet::Scene;

void PodBehaviour::StartImpl()
{
	// Load the pod scene if not already loaded
	if (m_podScene.expired())
	{
		m_podScene = GetSceneManager()->GetScene("PodScene");
		if (m_podScene.expired())
		{
			GetSceneManager()->LoadFromFile(ROOT_PATH "/GameData/Scenes/PodScene.json");
			m_podScene = GetSceneManager()->GetScene("PodScene");
		}
	}
}

void PodBehaviour::PreUpdateImpl()
{
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
#ifndef _GAME

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

std::weak_ptr<PodComponent> PodBehaviour::GetPodInScene(const std::string &p_sceneName)
{
	// Try to find the scene by name, if failed, load the scene

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

	GetSceneManager()->LoadFromFile(ROOT_PATH "/GameData/Scenes/" + p_sceneName + ".json");
	auto scene = GetSceneManager()->GetScene(p_sceneName);

	return GetPodInScene(scene);
}

void PodBehaviour::BeginTransition(const std::string &p_targetSceneName, std::weak_ptr<Scene> p_currentScene)
{
	if (m_transitionTimer > 0.0f)
	{
		return; // Already performing a transition
	}

	if (p_currentScene.expired())
	{
		return; // Invalid scenes
	}

	auto podInTargetScene = GetPodInScene(p_targetSceneName).lock();
	auto targetScene = GetSceneManager()->GetScene(p_targetSceneName);

	if (!podInTargetScene)
	{
		return; // No pod in the target scene, cannot transition
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
		GetSceneManager()->DeactivateScene(currentSceneShared->GetName());
	}

	if (auto podSceneShared = m_podScene.lock())
	{
		GetSceneManager()->ActivateScene(podSceneShared->GetName());
	}

	m_targetScene = targetScene;
	m_transitionTimer = m_transitionTime; // Start the transition timer
}
