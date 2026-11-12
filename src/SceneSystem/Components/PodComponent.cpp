#include "PodComponent.hpp"
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Behaviours/PodBehaviour.hpp>

using namespace Droplet::Scene;

PodComponent::~PodComponent()
{
	if (auto podBehaviourShared = m_podBehaviour.lock())
	{
		podBehaviourShared->RemovePodComponent(this);
	}
}

void PodComponent::StartImpl()
{
	// Search for the PodBehaviour. If none was found, create one.
	auto scene = GetOwner()->GetScene();

	auto podBehaviour = scene->GetBehaviour<PodBehaviour>();
	if (!podBehaviour)
	{
		podBehaviour = std::dynamic_pointer_cast<PodBehaviour>(scene->StartBehaviour("PodBehaviour").lock());
	}

	m_podBehaviour = podBehaviour;

	std::shared_ptr<PodComponent> sharedThis = std::dynamic_pointer_cast<PodComponent>(shared_from_this());
	podBehaviour->AddPodComponent(sharedThis);
}

void PodComponent::RenderUIImpl()
{
#ifndef _GAME



#endif
}
