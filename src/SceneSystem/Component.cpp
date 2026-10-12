#include "Component.hpp"
#include <Node.hpp>
#include <ImGui/imgui.h>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;

std::shared_ptr<NodeBounds> Droplet::Scene::Component::GetBounds() const
{
    // TODO: Implement GetBounds()
    return std::shared_ptr<NodeBounds>();
}

Droplet::Scene::Component::Component()
{
	ZoneScoped;

}

void Component::RenderUI()
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

    // TODO: Set active, delete

    RenderUIImpl();
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
    compJson["type"] = ""; 

	compJson["data"] = SerializeImpl();

    return SerializeImpl();
}

void Component::Deserialize(nlohmann::json p_compJson)
{
	ZoneScoped;
	ZoneText(GetTypeName().data(), GetTypeName().size());

	nlohmann::json dataJson = p_compJson["data"];

	DeserializeImpl(dataJson);
}
