#include "Component.hpp"
#include <Node.hpp>
#include <ImGui/imgui.h>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <SerializationContext.hpp>
using namespace Droplet::Scene;

Component::Component()
{
	ZoneScoped;

}

void Component::RenderUI()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

    // TODO: Set active, delete

	ImGuiChildFlags childFlags = ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY;
	if (ImGui::BeginChild("ComponentUI", ImVec2(0,0), childFlags))
	{
		if (ImGui::Button("Remove Component"))
		{
			m_owner.lock()->RemoveComponent(this);
		}

		ImGui::Dummy(ImVec2(0, 8.0f));

		ImGui::Separator();

		RenderUIImpl();
	}

	ImGui::EndChild();
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
}

void Component::SetOwner(std::shared_ptr<Node> p_owner)
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
	m_active = p_active;
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

    return compJson;
}

void Component::Deserialize(const nlohmann::json p_compJson)
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	if (!p_compJson.contains("data"))
	{
		throw std::runtime_error("Component is missing field 'data'");
	}
	nlohmann::json dataJson = p_compJson["data"];

	DeserializeImpl(dataJson);
}

void Droplet::Scene::Component::ResolveReferences([[maybe_unused]]SerializationContext &p_context)
{
}
