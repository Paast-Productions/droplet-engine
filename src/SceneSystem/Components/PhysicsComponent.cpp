#include "PhysicsComponent.hpp"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

using namespace Droplet::Scene;

void Droplet::PhysicsComponent::Initialize(Physics *p_physics, Scene::Transform *p_transform)
{
	m_physics = p_physics;
	m_transform = p_transform;
}

void Droplet::PhysicsComponent::Update(float p_deltaTime)
{
	PushTransformToPhysics();
}

void Droplet::PhysicsComponent::LateUpdate(float p_deltaTime)
{
	PullTransformFromPhysics();
}

void Droplet::PhysicsComponent::SetBoxCollider(const JPH::Vec3 p_halfExtents)
{
	if (m_hasShape)
	{
		return;
	}

	JPH::BoxShapeSettings boxSettings(p_halfExtents);
	CreateCollider(boxSettings);
}

void Droplet::PhysicsComponent::SetBoxCollider(const glm::vec3 p_halfExtents)
{
	if (m_hasShape)
	{
		return;
	}

	SetBoxCollider(JPH::Vec3(p_halfExtents.x, p_halfExtents.y, p_halfExtents.z));
}

void Droplet::PhysicsComponent::SetSphereCollider(const float p_radius)
{
	if (m_hasShape)
	{
		return;
	}

	JPH::SphereShapeSettings sphereSettings(p_radius);
	CreateCollider(sphereSettings);
}

void Droplet::PhysicsComponent::SetCapsuleCollider(const float p_HalfHeightOfCylinder, const float p_radius)
{
	if (m_hasShape)
	{
		return;
	}

	JPH::CapsuleShapeSettings capsuleSettings(p_HalfHeightOfCylinder, p_radius);
	CreateCollider(capsuleSettings);
}

void Droplet::PhysicsComponent::SetMeshCollider(const Droplet::MeshResource *p_mesh)
{
	if (m_hasShape || p_mesh == nullptr)
	{
		return;
	}

	// Make a Jolt array for vertices
	const std::size_t C_BYTES_PER_VERTEX = p_mesh->GetVertexByteSize();
	const std::size_t C_VERTEX_COUNT = p_mesh->GetVertexData().size() / C_BYTES_PER_VERTEX;
	
	JPH::VertexList joltVertices{};
	joltVertices.reserve(C_VERTEX_COUNT);
	
	std::vector<std::byte> vertexData = p_mesh->GetVertexData();
	for (std::size_t i = 0; i < C_VERTEX_COUNT; i++)
	{
		JPH::Float3 pos;
		std::memcpy(&pos, vertexData.data() + i * C_BYTES_PER_VERTEX, sizeof(JPH::Float3));
		
		joltVertices.emplace_back(pos);
	}

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
	m_physics->GetBodyInterface()->SetMotionType(m_bodyID, p_motionType, JPH::EActivation::Activate);
}

void Droplet::PhysicsComponent::SetObjectLayer(const Layers p_objectLayer)
{
	m_physics->GetBodyInterface()->SetObjectLayer(m_bodyID, p_objectLayer);
}

void Droplet::PhysicsComponent::CreateCollider(JPH::ShapeSettings &p_shapeSettings)
{
	JPH::ShapeSettings::ShapeResult result = p_shapeSettings.Create();

	if (result.HasError())
	{
		throw std::runtime_error(result.GetError().c_str());
	}

	m_shape = result.Get();
	JPH::BodyCreationSettings bodySettings(m_shape, GLMtoJPH(m_transform->GetPosition(Transform::Space::World)), 
		GLMtoJPH(m_transform->GetRotation(Transform::Space::World)), JPH::EMotionType::Static, Layers::NON_MOVING);
	bodySettings.mAllowDynamicOrKinematic = true;
	bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
	bodySettings.mMassPropertiesOverride = JPH::MassProperties(1.0f);

	JPH::Body *body = m_physics->GetBodyInterface()->CreateBody(bodySettings);
	m_bodyID = body->GetID();

	m_physics->GetBodyInterface()->AddBody(m_bodyID, JPH::EActivation::Activate);

	m_hasShape = true;
}

void Droplet::PhysicsComponent::PushTransformToPhysics()
{
	if (!m_hasShape)
	{
		return;
	}

	// TODO: Do this check outside this function
	//if (m_transform.HasBeenChanged())
	//{
	//	PushTransformToPhysics();
	//}

	const JPH::Vec3 pos = GLMtoJPH(m_transform->GetPosition(Transform::Space::World));
	const JPH::Quat rot = GLMtoJPH(m_transform->GetRotation(Transform::Space::World));

	m_physics->GetBodyInterface()->SetPositionAndRotation(m_bodyID, pos, rot, JPH::EActivation::Activate);
}

void Droplet::PhysicsComponent::PullTransformFromPhysics()
{
	if (!m_hasShape)
	{
		return;
	}

	const JPH::RMat44 joltTransform = m_physics->GetBodyInterface()->GetWorldTransform(m_bodyID);

	const glm::vec3 pos = JPHtoGLM(joltTransform.GetTranslation());
	const glm::quat rot = JPHtoGLM(joltTransform.GetQuaternion());

	m_transform->SetPosition(pos);
	m_transform->SetRotation(rot);
}
