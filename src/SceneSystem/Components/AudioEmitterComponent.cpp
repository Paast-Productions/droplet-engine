#include "AudioEmitterComponent.hpp"

Droplet::Scene::AudioEmitterComponent::AudioEmitterComponent()
{
}

void Droplet::Scene::AudioEmitterComponent::StartImpl()
{

}

void Droplet::Scene::AudioEmitterComponent::UpdateImpl(float deltaTime)
{
}

void Droplet::Scene::AudioEmitterComponent::RenderUIImpl()
{
}

nlohmann::json Droplet::Scene::AudioEmitterComponent::SerializeImpl()
{
	// Member variables that needs to be saved gets added here
	return nlohmann::json();
}

void Droplet::Scene::AudioEmitterComponent::DeserializeImpl(nlohmann::json p_compJson)
{
}
