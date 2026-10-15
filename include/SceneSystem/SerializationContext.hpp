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
		void RegisterNode(std::uint64_t p_nodeID,  Node* p_node);
		void RegisterCopmonent(std::uint64_t p_nodeID, std::uint64_t p_componentID, Component* p_component);

		Node *FindNode(std::uint64_t p_nodeID);
		Component *FindComponent(std::uint64_t p_nodeID, std::uint64_t p_componentID);

		void ResolveAllReferences();

	private:
		std::unordered_map<std::uint64_t, Node *> m_nodes;
		std::unordered_map<std::uint64_t, std::unordered_map<std::uint64_t, Component *>> m_components;

		std::vector<Component *> m_registeredComponents;
	};
};
