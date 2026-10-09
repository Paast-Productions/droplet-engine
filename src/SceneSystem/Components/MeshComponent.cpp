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

const std::string &MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}

const std::weak_ptr<Droplet::MeshResource> MeshComponent::GetMeshResource() const
{
    // TODO
    return std::weak_ptr<MeshResource>();
}

nlohmann::json MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    // TODO
    json["filepath"] = GetMeshPath();

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] nlohmann::json p_compJson)
{
    // TODO
}
