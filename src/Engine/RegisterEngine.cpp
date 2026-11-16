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
		Scene::ComponentRegistry::RegisterComponent("MeshComponent", []() { return std::make_unique<Scene::MeshComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("ScriptComponent", []() { return std::make_unique<Scene::ScriptComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("AudioListenerComponent", []() { return std::make_unique<Scene::AudioListenerComponent>();});
		Scene::ComponentRegistry::RegisterComponent("AudioEmitterComponent", []() { return std::make_unique<Scene::AudioEmitterComponent>();});
		Scene::ComponentRegistry::RegisterComponent("PhysicsComponent", []() { return std::make_unique<Scene::PhysicsComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("CameraComponent", []() { return std::make_unique<Scene::CameraComponent>(); });
		Scene::ComponentRegistry::RegisterComponent("SkeletonComponent", []() { return std::make_unique<Scene::SkeletonComponent>(); });
	}

	void RegisterEngineBehaviours()
	{
		Scene::BehaviourRegistry::RegisterBehaviour("ScriptBehaviour", []() { return std::make_unique<Scene::ScriptBehaviour>(); });
	}
}