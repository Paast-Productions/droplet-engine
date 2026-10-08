#include "MeshComponent.hpp"

using namespace Droplet::Scene;

MeshComponent::MeshComponent(const std::string &p_meshPath)
    : m_meshPath((p_meshPath))
{
}

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
	//TODO: will be inplemted later when the mesh system is implemented
}

void Droplet::Scene::MeshComponent::Initilize(const std::string &p_meshPath)
{
    m_meshPath = p_meshPath;
}

const std::string &MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}

nlohmann::json Droplet::Scene::MeshComponent::SerializeImpl()
{
    nlohmann::json json;
    json["filepath"] = GetMeshPath();

    return json;
}

void Droplet::Scene::MeshComponent::DeserializeImpl([[maybe_unused]] nlohmann::json p_compJson)
{
}
