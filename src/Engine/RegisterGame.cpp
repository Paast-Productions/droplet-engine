#include <RegisterGame.hpp>

#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>

#include <SceneSystem/Components/HealthComponent.hpp>
#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Components/TriggerComponent.hpp>
#include <SceneSystem/Components/HazardComponent.hpp>
#include <SceneSystem/Components/InteractableComponent.hpp>
#include <SceneSystem/Components/PodComponent.hpp>
#include <SceneSystem/Components/LadderComponent.hpp>
#include <SceneSystem/Components/CollectibleTypeComponent.hpp>
#include <SceneSystem/Components/TrackerComponent.hpp>

#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
#include <SceneSystem/Behaviours/PodBehaviour.hpp>
#include <SceneSystem/Behaviours/LadderBehaviour.hpp>

namespace Droplet::Game
{
	void RegisterGameComponents()
	{

		Scene::ComponentRegistry::RegisterComponent("HealthComponent", []() { return std::make_unique<Scene::HealthComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("PlayerComponent", []() { return std::make_unique<Scene::PlayerComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("TriggerComponent", []() { return std::make_unique<Scene::TriggerComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("HazardComponent", []() { return std::make_unique<Scene::HazardComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("InteractableComponent", []() { return std::make_unique<Scene::InteractableComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("PodComponent", []() { return std::make_unique<Scene::PodComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("LadderComponent", []() { return std::make_unique<Droplet::Scene::LadderComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("CollectibleTypeComponent", []() { return std::make_unique<Scene::CollectibleTypeComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("TrackerComponent", []() { return std::make_unique<Scene::TrackerComponent>(); });
	}

	void RegisterGameBehaviours()
	{
		Droplet::Scene::BehaviourRegistry::RegisterBehaviour("PlayerBehaviour", []() { return std::make_unique<Droplet::Scene::PlayerBehaviour>(); });
		Droplet::Scene::BehaviourRegistry::RegisterBehaviour("PodBehaviour", []() { return std::make_unique<Droplet::Scene::PodBehaviour>(); });
		Droplet::Scene::BehaviourRegistry::RegisterBehaviour("LadderBehaviour", []() { return std::make_unique<Droplet::Scene::LadderBehaviour>(); });
	}
}
 