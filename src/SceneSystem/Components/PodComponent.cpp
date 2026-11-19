#include "PodComponent.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>
#include <SceneSystem/Behaviours/PodBehaviour.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <Editor/NodeSlot.hpp>
#include <GameInput.hpp>
#include <Time.hpp>

using namespace Droplet::Scene;

PodComponent::~PodComponent()
{
	if (auto podBehaviourShared = m_podBehaviour.lock())
	{
		podBehaviourShared->RemovePodComponent(this);
	}
}

void PodComponent::ConstructImpl()
{
	// Search for the PodBehaviour. If none was found, create one.
	auto scene = GetOwner()->GetScene();

	auto podBehaviour = scene->GetBehaviour<PodBehaviour>();
	if (!podBehaviour)
	{
		podBehaviour = std::dynamic_pointer_cast<PodBehaviour>(scene->StartBehaviour("PodBehaviour").lock());
	}

	m_podBehaviour = podBehaviour;

	std::shared_ptr<PodComponent> sharedThis = std::dynamic_pointer_cast<PodComponent>(shared_from_this());
	podBehaviour->AddPodComponent(sharedThis);
}

void PodComponent::UpdateImpl()
{
	if (m_transitionState == Idle)
	{
		GameInput input = GameInput::Get();

		if (auto button = m_transitionButton.GetTarget())
		{
			// If 
			//	1. The transition key was pressed this frame
			//  2. The camera raycasts to the transition button
			//  3. The button is within reach
			// Then trigger the transition

			bool doTransition = true;

			if (!input.KeyPressed(m_transitionButtonKey))
			{
				doTransition = false;
			}

			if (doTransition)
			{
				std::shared_ptr<Node> casterNode = nullptr;

				if (m_playerNode.GetTarget())
				{
					casterNode = m_playerNode.GetTarget();
				}
				else if (m_droneNode.GetTarget())
				{
					casterNode = m_droneNode.GetTarget();
				}

				std::shared_ptr<CameraComponent> camera = nullptr;
				if (casterNode)
				{
					camera = casterNode->GetComponentInChildren<CameraComponent>(true);
				}

				if (!camera)
				{
					doTransition = false;
				}
				else
				{
					Math::Ray ray = camera->GetCamera().GetRay();

					Math::RayHit hit = button->GetBounds()->Raycast(ray, button->GetTransform().GetMatrix(Transform::Space::World));

					if (!hit.didHit || hit.distance > m_transitionButtonReach)
					{
						doTransition = false;
					}
				}
			}

			if (doTransition)
			{
				OnTransitionFromThisPod();
			}
		}
	}
	else
	{
		m_doorTimer -= Time::Get().GetDeltaTime();

		if (m_transitionState == ClosingDoor)
		{
			if (m_doorTimer <= 0.0f)
			{
				m_transitionState = Idle;
				if (auto podBehaviourShared = m_podBehaviour.lock())
				{
					podBehaviourShared->BeginTransition(m_targetSceneName, GetOwner()->GetScene());
				}
			}
		}
		else if (m_transitionState == OpeningDoor)
		{
			if (m_doorTimer <= 0.0f)
			{
				m_transitionState = Idle;
			}
		}
	}
}

void PodComponent::RenderUIImpl()
{
#ifndef _GAME

	float buttonWidth = ImGui::GetContentRegionAvail().x;

	std::weak_ptr<Node> playerNode = m_playerNode.GetTarget();
	if (Editor::NodeSlot("Player", playerNode, ImVec2(buttonWidth, 0)))
	{
		m_playerNode.SetTarget(playerNode.lock());
	}

	std::weak_ptr<Node> droneNode = m_droneNode.GetTarget();
	if (Editor::NodeSlot("Drone", droneNode, ImVec2(buttonWidth, 0)))
	{
		m_droneNode.SetTarget(droneNode.lock());
	}

	std::weak_ptr<Node> buttonNode = m_transitionButton.GetTarget();
	if (Editor::NodeSlot("Button", buttonNode, ImVec2(buttonWidth, 0)))
	{
		m_transitionButton.SetTarget(buttonNode.lock());
	}

	std::weak_ptr<Node> doorNode = m_doorNode.GetTarget();
	if (Editor::NodeSlot("Door", doorNode, ImVec2(buttonWidth, 0)))
	{
		m_doorNode.SetTarget(doorNode.lock());
	}

	ImGui::InputText("Target Scene Name", &m_targetSceneName);

	ImGui::DragFloat("Button Reach", &m_transitionButtonReach, 0.05f, 0.2f);

	ImGui::Checkbox("Has Transition Period", &m_hasTransitionPeriod);

	if (m_hasTransitionPeriod)
	{
		ImGui::DragFloat("Time to Close Door", &m_timeToCloseDoor, 0.05f, 0.1f);
		ImGui::DragFloat("Time to Open Door", &m_timeToOpenDoor, 0.05f, 0.1f);
		ImGui::DragFloat("Buffer Time", &m_bufferTime, 0.05f, 0.0f);
	}

	if (ImGui::Button("Transition"))
	{
		OnTransitionFromThisPod();
	}
#endif
}

nlohmann::json PodComponent::SerializeImpl()
{
	nlohmann::json json;

	json["targetSceneName"] = m_targetSceneName;

	if (m_playerNode.GetTarget())
	{
		json["player"] = m_playerNode.GetTarget()->GetID();
	}

	if (m_droneNode.GetTarget())
	{
		json["drone"] = m_droneNode.GetTarget()->GetID();
	}

	if (m_transitionButton.GetTarget())
	{
		json["button"] = m_transitionButton.GetTarget()->GetID();
	}

	if (m_doorNode.GetTarget())
	{
		json["door"] = m_doorNode.GetTarget()->GetID();
	}

	json["reach"] = m_transitionButtonReach;

	json["hasTransitionPeriod"] = m_hasTransitionPeriod;

	if (m_hasTransitionPeriod)
	{
		json["timeToCloseDoor"] = m_timeToCloseDoor;
		json["timeToOpenDoor"] = m_timeToOpenDoor;
		json["bufferTime"] = m_bufferTime;
	}

	return json;
}

void PodComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("targetSceneName"))
	{
		m_targetSceneName = p_compJson["targetSceneName"].get<std::string>();
	}

	if (p_compJson.contains("player"))
	{
		SceneObjectID playerID;
		playerID.nodeID = p_compJson["player"].get<uint64_t>();
		m_playerNode.SetSerializedID(playerID);
	}

	if (p_compJson.contains("drone"))
	{
		SceneObjectID droneID;
		droneID.nodeID = p_compJson["drone"].get<uint64_t>();
		m_droneNode.SetSerializedID(droneID);
	}

	if (p_compJson.contains("button"))
	{
		SceneObjectID buttonID;
		buttonID.nodeID = p_compJson["button"].get<uint64_t>();
		m_transitionButton.SetSerializedID(buttonID);
	}

	if (p_compJson.contains("door"))
	{
		SceneObjectID doorID;
		doorID.nodeID = p_compJson["door"].get<uint64_t>();
		m_doorNode.SetSerializedID(doorID);
	}

	if (p_compJson.contains("reach"))
	{
		m_transitionButtonReach = p_compJson["reach"].get<float>();
	}

	if (p_compJson.contains("hasTransitionPeriod"))
	{
		m_hasTransitionPeriod = p_compJson["hasTransitionPeriod"].get<bool>();
	}

	if (p_compJson.contains("timeToCloseDoor"))
	{
		m_timeToCloseDoor = p_compJson["timeToCloseDoor"].get<float>();
	}

	if (p_compJson.contains("timeToOpenDoor"))
	{
		m_timeToOpenDoor = p_compJson["timeToOpenDoor"].get<float>();
	}

	if (p_compJson.contains("bufferTime"))
	{
		m_bufferTime = p_compJson["bufferTime"].get<float>();
	}
}

void PodComponent::ResolveReferences(SerializationContext &p_context)
{
	auto player = p_context.FindNode(m_playerNode.serializedID.nodeID);
	if (player)
	{
		m_playerNode.SetTarget(player);
	}

	auto drone = p_context.FindNode(m_droneNode.serializedID.nodeID);
	if (drone)
	{
		m_droneNode.SetTarget(drone);
	}

	auto button = p_context.FindNode(m_transitionButton.serializedID.nodeID);
	if (button)
	{
		m_transitionButton.SetTarget(button);
	}

	auto door = p_context.FindNode(m_doorNode.serializedID.nodeID);
	if (door)
	{
		m_doorNode.SetTarget(door);
	}
}

void PodComponent::OnTransitionToThisPod()
{
	if (!m_hasTransitionPeriod)
	{
		return;
	}

	m_transitionState = OpeningDoor;
	m_doorTimer = m_timeToOpenDoor + m_bufferTime;

	if (!m_doorNode.GetTarget())
	{
		return;
	}

	std::weak_ptr<ScriptComponent> doorScriptComponent = m_doorNode.GetTarget()->GetComponent<ScriptComponent>();
	if (auto doorScript = doorScriptComponent.lock())
	{
		doorScript->Call("OpenDoor", m_timeToOpenDoor);
	}
}

void PodComponent::OnTransitionFromThisPod()
{
	if (!m_hasTransitionPeriod)
	{
		if (auto podBehaviourShared = m_podBehaviour.lock())
		{
			podBehaviourShared->BeginTransition(m_targetSceneName, GetOwner()->GetScene());
		}

		return;
	}

	m_transitionState = ClosingDoor;
	m_doorTimer = m_timeToCloseDoor + m_bufferTime;

	std::weak_ptr<ScriptComponent> doorScriptComponent = m_doorNode.GetTarget()->GetComponent<ScriptComponent>();
	if (auto doorScript = doorScriptComponent.lock())
	{
		doorScript->Call("CloseDoor", m_timeToCloseDoor);
	}
}
