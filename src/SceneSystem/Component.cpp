#include "Component.hpp"
#include <Node.hpp>

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
    // TODO: implement component wrapper UI

    RenderUIImpl();

    // TODO: implement component wrapper UI
}

std::shared_ptr<Node> Component::GetOwner() const
{
    return m_owner.lock();
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
