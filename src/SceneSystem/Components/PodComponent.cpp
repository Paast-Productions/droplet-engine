#include "PodComponent.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>
#include <SceneSystem/Behaviours/PodBehaviour.hpp>
#include <Editor/NodeSlot.hpp>

using namespace Droplet::Scene;

PodComponent::~PodComponent()
{
	if (auto podBehaviourShared = m_podBehaviour.lock())
	{
		podBehaviourShared->RemovePodComponent(this);
	}
}

void PodComponent::StartImpl()
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

#endif
}

nlohmann::json PodComponent::SerializeImpl()
{
	nlohmann::json json;

	if (m_playerNode.GetTarget())
	{
		json["player"] = m_playerNode.GetTarget()->GetID();
	}

	if (m_droneNode.GetTarget())
	{
		json["drone"] = m_droneNode.GetTarget()->GetID();
	}

	return json;
}

void PodComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("player"))
	{
		SceneObjectID playerID;
		playerID.nodeID = p_compJson["player"];
		m_playerNode.SetSerializedID(playerID);
	}

	if (p_compJson.contains("drone"))
	{
		SceneObjectID droneID;
		droneID.nodeID = p_compJson["drone"];
		m_droneNode.SetSerializedID(droneID);
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
}
