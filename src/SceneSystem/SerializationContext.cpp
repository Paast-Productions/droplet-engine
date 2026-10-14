#include "SceneSystem/SerializationContext.hpp"

using namespace Droplet::Scene;


void SerializationContext::RegisterNode(std::uint32_t p_nodeID, Node *p_node)
{
	if (p_nodeID == 0 || p_node == nullptr)
	{
		throw std::invalid_argument("Invalid node registration");
	}

	const auto result = m_nodes.emplace(p_nodeID, p_node);

	if (!result.second)
	{
		throw std::runtime_error("Duplicate node ID");
	}
}

void Droplet::Scene::SerializationContext::RegisterCopmonent(std::uint32_t p_nodeID, std::uint32_t p_componentID, Component *p_component)
{
	if (p_nodeID == 0 || p_componentID == 0 || p_component == nullptr)
	{
		throw std::invalid_argument("Invalid compoonent registration");
	}

	const auto result = m_components[p_nodeID].emplace(p_componentID, p_component);
	if (!result.second)
	{
		throw std::runtime_error("Duplicate Component ID on node");
	}
}

Node *Droplet::Scene::SerializationContext::FindNode(std::uint32_t p_nodeID)
{
	const auto it = m_nodes.find(p_nodeID);

	if (it == m_nodes.end())
	{
		return nullptr;
	}
	return it->second;
}

Component *Droplet::Scene::SerializationContext::FindComponent(std::uint32_t p_nodeID, std::uint32_t p_componentID)
{
	const auto nodeIt = m_components.find(p_nodeID);

	if (nodeIt == m_components.end())
	{
		return nullptr;
	}

	const auto componentIt = nodeIt->second.find(p_componentID);
	if (componentIt == nodeIt->second.end())
	{
		return nullptr;
	}

	return componentIt->second;
}
