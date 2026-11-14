#include "SceneSystem/Components/AudioListenerComponent.hpp"
#include "Transform.hpp"
#include "SceneSystem/Node.hpp"
#include "SceneSystem/Components/PhysicsComponent.hpp"
#include <ImGui/imgui.h>

Droplet::Scene::AudioListenerComponent::AudioListenerComponent()
{
    Audio::AudioSystem::GetInstance().RegisterListener(this);
}

Droplet::Scene::AudioListenerComponent::~AudioListenerComponent()
{
    Audio::AudioSystem::GetInstance().UnregisterListener(this);
}

glm::vec3 Droplet::Scene::AudioListenerComponent::GetWorldPosition() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
	}

    Transform &transformComp = GetOwner()->GetTransform();
    return transformComp.GetPosition(Transform::Space::World);
}

glm::vec3 Droplet::Scene::AudioListenerComponent::GetForward() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }

    Transform& transformComp = GetOwner()->GetTransform();
    return transformComp.GetForward(Transform::Space::World);
}

glm::vec3 Droplet::Scene::AudioListenerComponent::GetUp() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 1.0f, 0.0f);
    }

    Transform& transformComp = GetOwner()->GetTransform();
    return transformComp.GetUp(Transform::Space::World);
}

glm::vec3 Droplet::Scene::AudioListenerComponent::GetVelocity() const
{
    if (!GetOwner())
    {
        return glm::vec3(0.0f, 0.0f, 1.0f);
    }

    std::shared_ptr<PhysicsComponent> physComp = GetOwner()->GetComponent<PhysicsComponent>();
    if (!physComp) 
    {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

    return physComp->GetVelocity();
}

void Droplet::Scene::AudioListenerComponent::RenderUIImpl()
{
    ImGui::Text("This is a listener");
}

nlohmann::json Droplet::Scene::AudioListenerComponent::SerializeImpl()
{
	// Member variables that needs to be saved gets added here
    nlohmann::json json;
    
	return json;
}

void Droplet::Scene::AudioListenerComponent::DeserializeImpl(nlohmann::json p_compJson)
{

}
