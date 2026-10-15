#include "PhysicsComponent.hpp"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Droplet::Scene;

void Droplet::PhysicsComponent::SetBoxCollider(const JPH::Vec3 p_halfExtents)
{
	if (!m_hasShape)
	{
		return;
	}

	JPH::BoxShapeSettings boxSettings(p_halfExtents);
	CreateCollider(boxSettings);
}

void Droplet::PhysicsComponent::SetBoxCollider(const glm::vec3 p_halfExtents)
{
	if (!m_hasShape)
	{
		return;
	}

	SetBoxCollider(JPH::Vec3(p_halfExtents.x, p_halfExtents.y, p_halfExtents.z));
}

void Droplet::PhysicsComponent::SetSphereCollider(const float p_radius)
{
	if (!m_hasShape)
	{
		return;
	}

	JPH::SphereShapeSettings sphereSettings(p_radius);
	CreateCollider(sphereSettings);
}

void Droplet::PhysicsComponent::SetCapsuleCollider(const float p_HalfHeightOfCylinder, const float p_radius)
{
	if (!m_hasShape)
	{
		return;
	}

	JPH::CapsuleShapeSettings capsuleSettings(p_HalfHeightOfCylinder, p_radius);
	CreateCollider(capsuleSettings);
}

void Droplet::PhysicsComponent::SetMeshCollider(const std::shared_ptr<Droplet::MeshResource> p_mesh)
{
	if (!m_hasShape || p_mesh == nullptr)
	{
		return;
	}

	// Make a Jolt array for vertices
	JPH::VertexList joltVertices;
	std::vector<std::byte> vertexData = p_mesh->GetVertexData();
	std::size_t nrOfVertices = vertexData.size() / p_mesh->GetVertexByteSize();
	joltVertices.reserve(nrOfVertices);
	std::memcpy(joltVertices.data(), vertexData.data(), vertexData.size());

	// Make a Jolt array for triangles based of indices
	JPH::IndexedTriangleList triangles;
	std::vector<std::uint32_t> indexData = p_mesh->GetIndexData();
	std::size_t nrOfIndices = indexData.size();
	for (std::size_t i = 0; i < nrOfIndices; i += 3)
	{
		triangles.emplace_back(
			indexData[i],
			indexData[i + 1],
			indexData[i + 2]
		);
	}

	JPH::MeshShapeSettings meshSettings(std::move(joltVertices), std::move(triangles));
	CreateCollider(meshSettings);
}

void Droplet::PhysicsComponent::SetMotionType(const JPH::EMotionType p_motionType)
{
	m_physics.GetBodyInterface()->SetMotionType(m_bodyID, p_motionType, JPH::EActivation::Activate);
}

void Droplet::PhysicsComponent::SetObjectLayer(const Layers p_objectLayer)
{
	m_physics.GetBodyInterface()->SetObjectLayer(m_bodyID, p_objectLayer);
}

void Droplet::PhysicsComponent::CreateCollider(JPH::ShapeSettings &p_shapeSettings)
{
	JPH::ShapeSettings::ShapeResult result = p_shapeSettings.Create();

	if (result.HasError())
	{
		throw std::runtime_error(result.GetError().c_str());
	}

	m_shape = result.Get();
	JPH::BodyCreationSettings bodySettings(m_shape, GLMtoJPH(m_transform.GetPosition(Transform::Space::World)), 
		GLMtoJPH(m_transform.GetRotation(Transform::Space::World)), JPH::EMotionType::Static, Layers::NON_MOVING);

	JPH::Body *body = m_physics.GetBodyInterface()->CreateBody(bodySettings);
	m_bodyID = body->GetID();

	m_physics.GetBodyInterface()->AddBody(m_bodyID, JPH::EActivation::Activate);

	m_hasShape = true;
}
