#include "SceneSystem/SerializationContext.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Component.hpp"

using namespace Droplet::Scene;


void SerializationContext::RegisterNode(std::uint64_t p_nodeID, std::shared_ptr<Node> p_node)
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

void Droplet::Scene::SerializationContext::RegisterCopmonent(std::uint64_t p_nodeID, std::uint64_t p_componentID, std::shared_ptr<Component> p_component)
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
	m_registeredComponents.push_back((p_component));
}

std::shared_ptr<Node> Droplet::Scene::SerializationContext::FindNode(std::uint64_t p_nodeID)
{
	const auto it = m_nodes.find(p_nodeID);
	if (it == m_nodes.end())
	{
		return nullptr;
	}
	return it->second.lock();
}

std::shared_ptr<Component> Droplet::Scene::SerializationContext::FindComponent(std::uint64_t p_nodeID, std::uint64_t p_componentID)
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

	return componentIt->second.lock();
}

void Droplet::Scene::SerializationContext::ResolveAllReferences()
{
	for (const std::weak_ptr<Component> &weakComponent : m_registeredComponents)
	{
		std::shared_ptr<Component> comp = weakComponent.lock();
		if (!comp)
		{
			continue;
		}

		comp->ResolveReferences(*this);
	}
	for (auto component : m_registeredComponents)
	{
		//componentResolveReferences(*this);
	}
}
