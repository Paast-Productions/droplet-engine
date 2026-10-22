#include "AudioListenerComponent.hpp"

Droplet::Scene::AudioListenerComponent::AudioListenerComponent()
{
}

void Droplet::Scene::AudioListenerComponent::StartImpl()
{
}

void Droplet::Scene::AudioListenerComponent::UpdateImpl(float deltaTime)
{
}

void Droplet::Scene::AudioListenerComponent::RenderUIImpl()
{
}

nlohmann::json Droplet::Scene::AudioListenerComponent::SerializeImpl()
{
	// Member variables that needs to be saved gets added here
	return nlohmann::json();
}

void Droplet::Scene::AudioListenerComponent::DeserializeImpl(nlohmann::json p_compJson)
{
}
