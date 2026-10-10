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

nlohmann::json MeshComponent::SerializeImpl()
{
    nlohmann::json json;

    json["filepath"] = GetMeshPath();

    return json;
}

void MeshComponent::DeserializeImpl([[maybe_unused]] const nlohmann::json p_compJson)
{
	// TODO: Implement deserialization logic for the mesh component.
}
