#include "MeshComponent.hpp"

MeshComponent::MeshComponent(std::string p_meshPath)
    : m_meshPath(std::move(p_meshPath))
{
}

void MeshComponent::Initialize()
{
    // Mesh loading will be implemented later.
}

void MeshComponent::Update([[maybe_unused]] float p_deltaTime)
{
    // Mesh-specific update logic will go here later.
}

const std::string& MeshComponent::GetMeshPath() const
{
    return m_meshPath;
}