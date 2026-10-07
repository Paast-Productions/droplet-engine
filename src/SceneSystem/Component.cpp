#include "Component.hpp"
#include <Node.hpp>
#include <ImGui/imgui.h>
#include <stdexcept>

using namespace Droplet::Scene;

void Component::SetOwner(std::shared_ptr<Node> p_owner)
{
    m_owner = p_owner;
}

Droplet::Scene::Component::Component()
{

}

void Component::RenderUI()
{
    using namespace ImGui;

    // TODO: Set active, delete

    RenderUIImpl();
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
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
	nlohmann::json dataJson = p_compJson["data"];

	DeserializeImpl(dataJson);
}
