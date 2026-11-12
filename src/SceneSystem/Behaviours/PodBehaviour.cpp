#include "PodBehaviour.hpp"
#include <SceneSystem/Scene.hpp>
#include <SceneSystem/Node.hpp>
#include <SceneSystem/Components/PodComponent.hpp>

using namespace Droplet::Scene;

void PodBehaviour::AddPodComponent(std::weak_ptr<PodComponent> podComponent)
{
	if (podComponent.expired())
	{
		return; // Invalid PodComponent
	}

	for (const auto &existingPodComponent : m_podComponents)
	{
		auto existingPodComponentShared = existingPodComponent.lock();
		if (!existingPodComponentShared)
		{
			continue; // Skip expired weak pointers
		}

		if (existingPodComponentShared == podComponent.lock())
		{
			return; // Already added
		}

		if (existingPodComponentShared->GetOwner()->GetScene() == podComponent.lock()->GetOwner()->GetScene())
		{
			return; // A PodComponent from the same scene is already registered
		}
	}

	m_podComponents.push_back(podComponent);
}

void PodBehaviour::RemovePodComponent(PodComponent *podComponent)
{
	if (!podComponent)
	{
		return; // Invalid PodComponent
	}

	auto it = std::remove_if(m_podComponents.begin(), m_podComponents.end(),
		[&podComponent](const std::weak_ptr<PodComponent> &existingPodComponent) {
			auto existingPodComponentShared = existingPodComponent.lock();
			if (!existingPodComponentShared)
			{
				return true; // Remove expired weak pointers
			}
			return existingPodComponentShared.get() == podComponent;
		});

	m_podComponents.erase(it, m_podComponents.end());
}

std::weak_ptr<PodComponent> PodBehaviour::GetPodInScene(std::weak_ptr<Scene> scene) const
{
	auto sceneShared = scene.lock();
	if (!sceneShared)
	{
		return std::weak_ptr<PodComponent>();
	}

	for (const auto &podComponent : m_podComponents)
	{
		auto podComponentShared = podComponent.lock();
		if (!podComponentShared)
		{
			continue;
		}

		if (podComponentShared->GetOwner()->GetScene() == sceneShared)
		{
			return podComponent;
		}
	}

	return std::weak_ptr<PodComponent>();
}

void PodBehaviour::RenderUIImpl()
{
#ifndef _GAME

	// Show all registered PodComponents along with the containing scene in a table.

	if (ImGui::BeginTable("PodComponentsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Scene");
		ImGui::TableSetupColumn("Node");

		for (const auto &podComponent : m_podComponents)
		{
			auto podComponentShared = podComponent.lock();
			if (!podComponentShared)
			{
				continue; // Skip expired weak pointers
			}

			auto scene = podComponentShared->GetOwner()->GetScene();
			if (!scene)
			{
				continue; // Skip if the scene is not valid
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text(scene->GetName().c_str());

			ImGui::TableSetColumnIndex(1);
			ImGui::Text(podComponentShared->GetOwner()->GetName().c_str());
		}

		ImGui::EndTable();
	}

#endif
}
