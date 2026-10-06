#pragma once
#include <json/json.hpp>
#include <SceneSystem/Scene.hpp>

#include <SceneSystem/Node.hpp>
#include "Components/ScriptComponent.hpp"
#include "Components/MeshComponent.hpp"
#include "Debug/Logger.hpp"

namespace Droplet::Scene
{
	class SceneSerializer
	{
	public:
		SceneSerializer() = default;

		nlohmann::json SerializeScene(const std::shared_ptr<Droplet::Scene::Scene> p_scene);

		void DeserializeScene(const nlohmann::json& p_json, SceneManager &p_sceneManager);

	private:

		nlohmann::json SerializeNode(const Droplet::Scene::Node& p_node);
		void DeserializeNode(const nlohmann::json& p_json, std::shared_ptr<Droplet::Scene::Node> p_parentNode, std::shared_ptr<Droplet::Scene::Scene> p_scene);

		void DeserializeComponent(const nlohmann::json &p_json, std::shared_ptr<Droplet::Scene::Node> p_node);

		nlohmann::json SerializeTransform(const Droplet::Scene::Transform& p_transform);
		void DeserializeTransform(const nlohmann::json& p_json, Droplet::Scene::Transform& p_transform);

	private:


	};
};
