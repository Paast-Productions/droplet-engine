#include "MeshComponent.hpp"

MeshComponent::MeshComponent(std::string p_meshPath)
    : m_meshPath(std::move(p_meshPath))
{
}

void MeshComponent::Initialize()
{
	//TODO: will be inplemted later when the mesh system is implemented
}

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
	//TODO: will be inplemted later when the mesh system is implemented
}

const std::string &MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}