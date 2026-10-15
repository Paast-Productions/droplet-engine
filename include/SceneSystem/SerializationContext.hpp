#pragma once

#include <json/json.hpp>

#include <SceneSystem/Scene.hpp>

#include "Debug/Logger.hpp"

namespace Droplet::Scene
{
	class Node;
	class Component;

	class SerializationContext
	{
	public:
		void RegisterNode(std::uint64_t p_nodeID,  std::shared_ptr<Node> p_node);
		void RegisterCopmonent(std::uint64_t p_nodeID, std::uint64_t p_componentID, std::shared_ptr<Component> p_component);

		std:: shared_ptr<Node> FindNode(std::uint64_t p_nodeID);
		std::shared_ptr<Component> FindComponent(std::uint64_t p_nodeID, std::uint64_t p_componentID);

		void ResolveAllReferences();

	private:
		std::unordered_map<std::uint64_t, std::weak_ptr<Node>> m_nodes;
		std::unordered_map<std::uint64_t, std::unordered_map<std::uint64_t, std::weak_ptr<Component>>> m_components;

		std::vector<std::weak_ptr<Component>> m_registeredComponents;
	};
};
