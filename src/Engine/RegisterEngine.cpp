#include <RegisterEngine.hpp>

#include <SceneSystem/ComponentRegistry.hpp>
#include <SceneSystem/Components/MeshComponent.hpp>
#include <SceneSystem/Components/SkeletonComponent.hpp>
#include <SceneSystem/Components/ScriptComponent.hpp>
#include <SceneSystem/Components/AudioEmitterComponent.hpp>
#include <SceneSystem/Components/AudioListenerComponent.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>
#include <SceneSystem/Components/CameraComponent.hpp>

#include <SceneSystem/BehaviourRegistry.hpp>
#include <SceneSystem/Behaviours/ScriptBehaviour.hpp>

namespace Droplet
{
	void RegisterEngineComponents()
	{
		Droplet::Scene::ComponentRegistry::RegisterComponent("MeshComponent", []() { return std::make_unique<Droplet::Scene::MeshComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("ScriptComponent", []() { return std::make_unique<Droplet::Scene::ScriptComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("AudioListenerComponent", []() { return std::make_unique<Droplet::Scene::AudioListenerComponent>();});
		Droplet::Scene::ComponentRegistry::RegisterComponent("AudioEmitterComponent", []() { return std::make_unique<Droplet::Scene::AudioEmitterComponent>();});
		Droplet::Scene::ComponentRegistry::RegisterComponent("PhysicsComponent", []() { return std::make_unique<Droplet::Scene::PhysicsComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("CameraComponent", []() { return std::make_unique<Droplet::Scene::CameraComponent>(); });
		Droplet::Scene::ComponentRegistry::RegisterComponent("SkeletonComponent", []() { return std::make_unique<Droplet::Scene::SkeletonComponent>(); });
	}
	void RegisterEngineBehaviours()
	{
		Droplet::Scene::BehaviourRegistry::RegisterBehaviour("ScriptBehaviour", []() { return std::make_unique<Droplet::Scene::ScriptBehaviour>(); });
	}
}