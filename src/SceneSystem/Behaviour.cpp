#include "Behaviour.hpp"

#include <ImGui/imgui.h>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;

void Behaviour::Construct()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	ConstructImpl();
}

void Behaviour::Start()
{
	if (IsSingleton())
	{
		// Ensure this is the only instance of this behaviour type in the scene.
		auto &behaviours = m_sceneManager->GetBehaviours();

		for (const auto &behaviour : behaviours)
		{
			if (behaviour->GetTypeName() != GetTypeName())
			{
				continue;
			}

			if (behaviour.get() != this)
			{
				m_sceneManager->ShutdownBehaviour(shared_from_this());
				return;
			}
		}
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	if (m_isStarted)
	{
		return;
	}

	if (!m_isActive)
	{
		return;
	}

	StartImpl();

	m_isStarted = true;
}

void Behaviour::PreUpdate()
{
	if (m_isStarted)
	{
		return;
	}

	if (!m_isActive)
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());


	PreUpdateImpl();
}

void Behaviour::PostUpdate()
{
	if (m_isStarted)
	{
		return;
	}

	if (!m_isActive)
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	PostUpdateImpl();
}

void Behaviour::RenderUI()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	// TODO: Set active, delete

	ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
	if (ImGui::BeginChild("ComponentUI", ImVec2(0, 0), childFlags))
	{

		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::Separator();

		RenderUIImpl();
	}

	ImGui::EndChild();
}

void Behaviour::SetSceneManager(SceneManager *sceneManager)
{
	m_sceneManager = sceneManager;
}

nlohmann::json Behaviour::Serialize()
{
	nlohmann::json behJson;

	behJson["type"] = GetTypeName();

	behJson["data"] = SerializeImpl();

	return SerializeImpl();
}

void Behaviour::Deserialize(const nlohmann::json p_behJson)
{
	const nlohmann::json dataJson = p_behJson["data"];

	DeserializeImpl(dataJson);
}