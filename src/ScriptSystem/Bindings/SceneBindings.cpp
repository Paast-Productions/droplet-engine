#include "SceneBindings.hpp"
#include <tracy/public/tracy/Tracy.hpp>
#include <LuaBindings.hpp>
#include <SceneSystem/Scene.hpp>
#include <vector>

using namespace Droplet;

void Script::Bindings::RegisterScenes(sol::state_view p_luaState, Scene::SceneManager &p_sceneManager)
{
	ZoneScoped;

	p_luaState.new_usertype<Scene::Scene>(
		"Scene",
		"RaycastNode",
		[](Scene::Scene &p_scene, const Math::Ray &p_ray)
		{
			Math::RayHit hit{};

			std::weak_ptr<Scene::Node> weakNode = p_scene.RaycastNode(p_ray, &hit);

			auto node = weakNode.lock();

			if (!node || !hit.didHit)
			{
				return std::make_tuple(std::shared_ptr<Scene::Node>{}, hit);
			}

			return std::make_tuple(node, hit);
		},
		"RaycastNodes",
		[](Scene::Scene &p_scene, const Math::Ray &p_ray)
		{
			std::vector<std::shared_ptr<Scene::Node>> nodes;

			for (const auto &weakNode : p_scene.RaycastNodes(p_ray))
			{
				if (auto node = weakNode.lock())
				{
					nodes.push_back(std::move(node));
				}
			}

			return sol::as_table(std::move(nodes));
		}
	);

	p_luaState.new_usertype<Scene::SceneManager>(
		"SceneManager",
		"LoadScene", &Droplet::Scene::SceneManager::LoadScene,
		"UnloadScene", &Droplet::Scene::SceneManager::UnloadScene,
		"ActivateScene", &Droplet::Scene::SceneManager::ActivateScene,
		"DeactivateScene", &Droplet::Scene::SceneManager::DeactivateScene,
		"GetScene", &Droplet::Scene::SceneManager::GetScene,
		"GetActiveScenes", &Droplet::Scene::SceneManager::GetActiveScenes
	);

	p_luaState["SceneManagerInstance"] = &p_sceneManager;

	LuaClassDefinition classDef;
	classDef.name = "SceneManager";

	classDef.functions =
	{
		{
			"GetName",
			"string",
			{}
		},
		{
			"GetTransform",
			"Transform",
			{}
		}
	};

	LuaBindings::PushLuaClassDefinition(classDef);
}
