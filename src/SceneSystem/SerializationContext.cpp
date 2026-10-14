#include "SceneSystem/SerializationContext.hpp"

using namespace Droplet::Scene;


void SerializationContext::RegisterNode(std::uint32_t p_nodeID, Node *p_node)
{

}

void Droplet::Scene::SerializationContext::RegisterCopmonent(std::uint32_t p_nodeID, std::uint32_t p_componentID, Component *p_component)
{
}

Node *Droplet::Scene::SerializationContext::FindNode(std::uint32_t p_nodeID)
{
	return nullptr;
}

Component *Droplet::Scene::SerializationContext::FindComponent(std::uint32_t p_nodeID, std::uint32_t p_componentID)
{
	return nullptr;
}
