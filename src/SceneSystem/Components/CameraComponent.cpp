#include "CameraComponent.hpp"

Droplet::Scene::CameraComponent::CameraComponent()
{
}

void Droplet::Scene::CameraComponent::StartImpl()
{
}

void Droplet::Scene::CameraComponent::UpdateImpl(float deltaTime)
{
}

void Droplet::Scene::CameraComponent::RenderUIImpl()
{
}

nlohmann::json Droplet::Scene::CameraComponent::SerializeImpl()
{
	return nlohmann::json();
}

void Droplet::Scene::CameraComponent::DeserializeImpl(nlohmann::json p_compJson)
{
}
