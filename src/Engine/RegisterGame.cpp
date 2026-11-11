#include <RegisterGame.hpp>

#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/BehaviourRegistry.hpp>

#include <SceneSystem/Components/HealthComponent.hpp>
#include <SceneSystem/Components/PlayerComponent.hpp>
#include <SceneSystem/Components/TriggerComponent.hpp>
#include <SceneSystem/Components/DamageComponent.hpp>
#include <SceneSystem/Components/InteractableComponent.hpp>

#include <SceneSystem/Behaviours/PlayerBehaviour.hpp>
namespace Droplet::Game
{
	void RegisterGameComponents()
	{
		Droplet::Scene::ComponentRegistry::RegisterComponent("HealthComponent", []() { return std::make_unique<Droplet::Scene::HealthComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("PlayerComponent", []() { return std::make_unique<Droplet::Scene::PlayerComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("TriggerComponent", []() { return std::make_unique<Droplet::Scene::TriggerComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("DamageComponent", []() { return std::make_unique<Droplet::Scene::DamageComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("InteractableComponent", []() { return std::make_unique<Droplet::Scene::InteractableComponent>(); });
	}
	void RegisterGameBehaviours()
	{
		Droplet::Scene::BehaviourRegistry::RegisterBehaviour("PlayerBehaviour", []() { return std::make_unique<Droplet::Scene::PlayerBehaviour>(); });
	}
}
