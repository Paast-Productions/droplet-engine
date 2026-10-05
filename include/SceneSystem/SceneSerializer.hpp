#pragma once
#include <json/json.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Component.hpp>
#include <SceneSystem/Node.hpp>


namespace Droplet::Scene
{
	class SceneSerializer
	{
	public:
		SceneSerializer() = default;

		nlohmann::json SerializeScene(const Droplet::Scene::Scene& p_scene);

		void DeserializeScene(const nlohmann::json& p_json, const Droplet::Scene::Scene& p_scene);

	private:

		nlohmann::json SerializeNode(const Droplet::Scene::Node& p_node);
		void DeserializeNode(const nlohmann::json& p_json,const Droplet::Scene::Node& p_node);


		nlohmann::json SerializeTransform(const Droplet::Scene::Transform& p_transform);
		void DeserializeTransform(const nlohmann::json& p_json, const Droplet::Scene::Transform& p_transform);

	private:


	};
};
