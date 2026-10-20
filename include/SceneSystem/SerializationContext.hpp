#pragma once

#include <json/json.hpp>

#include <SceneSystem/Scene.hpp>

#include "Debug/Logger.hpp"

namespace Droplet::Scene
{
	class Node;
	class Component;

	/// @brief Allows components to have references
	/// The serialization contexts maps newly deserialized nodes and components.
	/// Allowing components to find the objects they were serializing, 
	/// since nodeID can be changed between loads depending on what was created before mapping the serialized IDs neccessary to keep track of the desired object
	/// 
	class SerializationContext
	{
	public:
		/// @brief Maps a node and it serialized ID
		/// 
		/// Registers a node and its ID to be mapped and referenced later
		/// 
		/// @param p_nodeID The serialized ID
		/// @param p_node The newly deserialized node
		void RegisterNode(std::uint64_t p_nodeID,  std::shared_ptr<Node> p_node);

		/// @brief Registers a compononent and maps it to be referenced during the resolve stage
		/// @param p_nodeID The nodeID the component is attached to
		/// @param p_componentID the component ID
		/// @param p_component The newly deserialized component
		void RegisterCopmonent(std::uint64_t p_nodeID, std::uint64_t p_componentID, std::shared_ptr<Component> p_component);


		/// @brief Finds a node in the map by its serialized ID
		/// @param p_nodeID The SerializedID
		/// @return returns the newly deserialized created node
		std:: shared_ptr<Node> FindNode(std::uint64_t p_nodeID);

		/// @brief Finds a component in the map by the nodeId and ComponentID
		/// @param p_nodeID serialized nodeID the component was attached to
		/// @param p_componentID The serialized component ID
		/// @return Returns the newly dezerialised component
		std::shared_ptr<Component> FindComponent(std::uint64_t p_nodeID, std::uint64_t p_componentID);

		/// @brief Iterates through all the deserialized components and calls resolve reference on them all
		void ResolveAllReferences();

	private:

		/// @brief Mapping of serialized ID and the created Node
		std::unordered_map<std::uint64_t, std::weak_ptr<Node>> m_nodes;
		/// @brief Mapping of the created Component, it's serialized ID, and the nodes serialized id the component is attached to
		std::unordered_map<std::uint64_t, std::unordered_map<std::uint64_t, std::weak_ptr<Component>>> m_components;
		/// @brief A vector of the created components this scene
		std::vector<std::weak_ptr<Component>> m_registeredComponents;
	};
};
