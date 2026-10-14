#include <SceneSystem/Physics/Physics.hpp>

#include <Time.hpp>

void Droplet::Physics::Initialize(const std::uint32_t p_maxBodies, const std::uint32_t p_numBodyMutexes,
	const std::uint32_t p_maxBodyPairs, const std::uint32_t p_maxContactConstraints)
{
	m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
	m_jobSystem.Init(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	m_physicsSystem.Init(p_maxBodies, p_numBodyMutexes, p_maxBodyPairs, p_maxContactConstraints,
		m_broadPhaseLayerInterface, m_objectVsBroadphaseLayerFilter, m_objectVsObjectLayerFilter);

	m_bodyInterface = &m_physicsSystem.GetBodyInterface();
}

void Droplet::Physics::Update()
{
	m_physicsSystem.Update(Droplet::Time::Get().GetDeltaTime(), 1, m_tempAllocator.get(), &m_jobSystem);
}

void Droplet::Physics::SetGravity(const JPH::Vec3 p_gravityVector)
{
	m_physicsSystem.SetGravity(p_gravityVector);
}

void Droplet::Physics::SetGravity(const glm::vec3 p_gravityVector)
{
	m_physicsSystem.SetGravity(GLMtoJPH(p_gravityVector));
}

JPH::BodyInterface *Droplet::Physics::GetBodyInterface()
{
	return m_bodyInterface;
}
