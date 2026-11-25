#include "TrackerComponent.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Serialization/SerializationContext.hpp>
#include <Editor/NodeSlot.hpp>

using namespace Droplet::Scene;

void TrackerComponent::LateUpdateImpl()
{
	// Get the target node to track
	std::weak_ptr<Node> targetNode = m_trackedNode.GetTarget();

	if (targetNode.expired())
	{
		return;
	}

	Transform &transform = GetTransform();
	Transform &targetTransform = targetNode.lock()->GetTransform();

	if (m_trackPosX || m_trackPosY || m_trackPosZ)
	{
		glm::vec3 newPosition = transform.GetPosition(Transform::Space::World);
		glm::vec3 trackedPos = targetTransform.GetPosition(Transform::Space::World);

		if (m_trackPosX)
		{
			newPosition.x = trackedPos.x;
		}
		if (m_trackPosY)
		{
			newPosition.y = trackedPos.y;
		}
		if (m_trackPosZ)
		{
			newPosition.z = trackedPos.z;
		}

		transform.SetPosition(newPosition, Transform::Space::World);
	}

	if (m_trackRotX || m_trackRotY || m_trackRotZ)
	{
		glm::vec3 newRotation = transform.GetEuler(Transform::Space::World);
		glm::vec3 trackedRot = targetTransform.GetEuler(Transform::Space::World);

		if (m_trackRotX)
		{
			newRotation.x = trackedRot.x;
		}
		else
		{
			newRotation.x = 0.0f;
		}

		if (m_trackRotY)
		{
			newRotation.y = trackedRot.y;
		}
		else
		{
			newRotation.y = 0.0f;
		}

		if (m_trackRotZ)
		{
			newRotation.z = trackedRot.z;
		}
		else
		{
			newRotation.z = 0.0f;
		}

		transform.SetEuler(newRotation, Transform::Space::World);
	}
}

void TrackerComponent::RenderUIImpl()
{
#ifdef _EDITOR

	float buttonWidth = ImGui::GetContentRegionAvail().x;

	std::weak_ptr<Node> trackedNode = m_trackedNode.GetTarget();
	if (Editor::NodeSlot("Target", trackedNode, ImVec2(buttonWidth, 0)))
	{
		m_trackedNode.SetTarget(trackedNode.lock());
	}

	ImGui::Checkbox("Track Position X", &m_trackPosX);
	ImGui::Checkbox("Track Position Y", &m_trackPosY);
	ImGui::Checkbox("Track Position Z", &m_trackPosZ);

	ImGui::Checkbox("Track Rotation X", &m_trackRotX);
	ImGui::Checkbox("Track Rotation Y", &m_trackRotY);
	ImGui::Checkbox("Track Rotation Z", &m_trackRotZ);

#endif
}

nlohmann::json TrackerComponent::SerializeImpl()
{
	nlohmann::json json;

	if (m_trackedNode.GetTarget())
	{
		json["target"] = m_trackedNode.GetTarget()->GetID();
	}

	json["trackPosX"] = m_trackPosX;
	json["trackPosY"] = m_trackPosY;
	json["trackPosZ"] = m_trackPosZ;

	json["trackRotX"] = m_trackRotX;
	json["trackRotY"] = m_trackRotY;
	json["trackRotZ"] = m_trackRotZ;

	return json;
}

void TrackerComponent::DeserializeImpl(const nlohmann::json p_compJson)
{
	if (p_compJson.contains("target"))
	{
		SceneObjectID targetID;
		targetID.nodeID = p_compJson["target"].get<uint64_t>();
		m_trackedNode.SetSerializedID(targetID);
	}

	if (p_compJson.contains("trackPosX"))
	{
		m_trackPosX = p_compJson["trackPosX"].get<bool>();
	}
	if (p_compJson.contains("trackPosY"))
	{
		m_trackPosY = p_compJson["trackPosY"].get<bool>();
	}
	if (p_compJson.contains("trackPosZ"))
	{
		m_trackPosZ = p_compJson["trackPosZ"].get<bool>();
	}

	if (p_compJson.contains("trackRotX"))
	{
		m_trackRotX = p_compJson["trackRotX"].get<bool>();
	}
	if (p_compJson.contains("trackRotY"))
	{
		m_trackRotY = p_compJson["trackRotY"].get<bool>();
	}
	if (p_compJson.contains("trackRotZ"))
	{
		m_trackRotZ = p_compJson["trackRotZ"].get<bool>();
	}
}

void TrackerComponent::ResolveReferences(SerializationContext &p_context)
{
	auto target = p_context.FindNode(m_trackedNode.serializedID.nodeID);
	if (target)
	{
		m_trackedNode.SetTarget(target);
	}
}
