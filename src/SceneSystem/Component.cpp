#include "Component.hpp"
#include <Node.hpp>
#include <ImGui/imgui.h>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SerializationContext.hpp>

using namespace Droplet::Scene;

void Component::Start()
{
	if (!IsActive())
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size()); 
	
	StartImpl();

	m_started = true;
}

void Component::Update()
{
	if (!IsActiveSelf())
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size()); 
	
	UpdateImpl();
}

void Component::LateUpdate()
{
	if (!IsActiveSelf())
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size()); 
	
	LateUpdateImpl();
}

void Component::FixedUpdate()
{
	if (!IsActiveSelf())
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	FixedUpdateImpl();
}

void Component::EditorUpdate()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	EditorUpdateImpl();
}

void Component::QueueRender()
{
	if (!IsActiveSelf())
	{
		return;
	}

	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	QueueRenderImpl();
}

void Component::GetRenderData(Graphics::RenderData &p_renderData)
{
	if (!IsActiveSelf())
	{
		return;
	}

	GetRenderDataImpl(p_renderData);
}

void Component::RenderUI()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
	if (ImGui::BeginChild("ComponentUI", ImVec2(0,0), childFlags))
	{
		bool active = IsActiveSelf();
		if (ImGui::Checkbox("Active", &active))
		{
			SetActiveSelf(active);
		}

		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::Separator();

		RenderUIImpl();
	}

	ImGui::EndChild();
}

std::shared_ptr<Node> Component::GetOwner() const // TODO: Make weak pointer
{
    return m_owner.lock();
}

void Component::SetOwner(std::shared_ptr<Node> p_owner) // TODO: Make weak pointer
{
    m_owner = p_owner;
}


bool Component::IsActive() const
{
	auto owner = m_owner.lock();
	if (!owner)
	{
		throw std::runtime_error("Component has no owner.");
	}

	return m_active && owner->IsActive();
}

void Component::SetActiveSelf(bool p_active)
{
	bool wasActive = IsActive();

	m_active = p_active;

	bool isActive = IsActive();

	if (wasActive != isActive)
	{
		ActiveChanged(isActive);
	}
}

void Component::ActiveChanged(const bool p_active)
{
	if (p_active && !m_started)
	{
		Start();
	}

	ActiveChangedImpl(p_active);
}

nlohmann::json Component::Serialize()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	nlohmann::json compJson;

    // TODO: Insert component type name.
	// This is how we know what type of component to create when deserializing.
    // Must be an exact match to the type name used in the ComponentRegistry.
    compJson["type"] = GetTypeName(); 
	compJson["id"] = GetID();
	compJson["data"] = SerializeImpl();
	compJson["active"] = IsActiveSelf();

    return compJson;
}

void Component::Deserialize(const nlohmann::json p_compJson)
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	if (p_compJson.contains("active"))
	{
		SetActiveSelf(p_compJson["active"].get<bool>());
	}

	if (!p_compJson.contains("data"))
	{
		throw std::runtime_error("Component is missing field 'data'");
	}
	nlohmann::json dataJson = p_compJson["data"];

	DeserializeImpl(dataJson);
}

void Component::ResolveReferences([[maybe_unused]]SerializationContext &p_context)
{
}
