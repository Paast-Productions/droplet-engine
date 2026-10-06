#pragma once

#include <json/json.hpp>
#include <SceneSystem/Scene.hpp>

#include <SceneSystem/Node.hpp>
#include "Components/ScriptComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Debug/Logger.hpp"

namespace Droplet::Scene
{

	/// @brief Serializes a scene
	///
	/// Scene Serializer is supposed to take in a scene and serialize it into a single json
	/// The serializer doesn't save it to a json that is the managers job
	/// Scene serializer also deserializes a scene, takes in a json and creates all the nodes and their info
	/// 
	class SceneSerializer
	{
	public:
		SceneSerializer() = default;

		/// @brief Serializes the scene and all of it's nodes
		/// @param p_scene The scene you want to serialize
		/// @return Returns a json for the IO manager to later write to a json file
		nlohmann::json SerializeScene(const std::shared_ptr<Droplet::Scene::Scene> p_scene);


		/// @brief Deserializes a scene, It creates all the components and their contents of the saved scene
		/// @param p_json The saved scene saved as a json
		/// @param p_sceneManager The scenemanager that is supposed to load the scene
		void DeserializeScene(const nlohmann::json& p_json,std::shared_ptr<Scene> p_scene);

	private:

		/// @brief Serializes a node
		/// @param p_node The node you want to serialize
		/// @return Returns a json to be stored in the scene json
		nlohmann::json SerializeNode(const Droplet::Scene::Node& p_node);

		/// @brief Deserializes a node, Creating it, its transform, and its children
		/// @param p_json the saved node saved as a json
		/// @param p_parentNode The parentnode that is the owner of this node
		/// @param p_scene The scene the node belongs to
		void DeserializeNode(const nlohmann::json& p_json, std::shared_ptr<Droplet::Scene::Node> p_parentNode, std::shared_ptr<Droplet::Scene::Scene> p_scene);


		/// @brief Desereliazes Components,
		/// It needs to check the components type to be able to either deserialize them here or call their own deserializations
		/// @param p_json The component saved as a json
		/// @param p_node the node this component is attached to
		void DeserializeComponent(const nlohmann::json &p_json, std::shared_ptr<Droplet::Scene::Node> p_node);

		/// @brief Serializes a transform, saves it as position rotation and scale
		/// @param p_transform The transform that is being serialized
		/// @return Returns a json to be stored in the corresponding owning node
		nlohmann::json SerializeTransform(const Droplet::Scene::Transform& p_transform);

		/// @brief Deserializes a transform, setting its values
		/// @param p_json The transform saved as a json
		/// @param p_transform the transform that has it's values changed
		void DeserializeTransform(const nlohmann::json& p_json, Droplet::Scene::Transform& p_transform);

	private:


	};
};
