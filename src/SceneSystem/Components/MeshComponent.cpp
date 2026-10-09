#include "MeshComponent.hpp"

using namespace Droplet::Scene;

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
	//TODO: will be inplemted later when the mesh system is implemented
}

const std::string &MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}

nlohmann::json Droplet::Scene::MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    json["type"] = "MeshComponent";
    json["filepath"] = GetMeshPath();

    return json;
}

void Droplet::Scene::MeshComponent::DeserializeImpl([[maybe_unused]] nlohmann::json p_compJson)
{
}
