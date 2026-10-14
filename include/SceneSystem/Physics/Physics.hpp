#pragma once

#include <Jolt/Jolt.h>

#include <SceneSystem/Physics/PhysicsHelpers.hpp>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <glm/glm.hpp>

namespace Droplet
{
	/// @brief A class managing all physics
	class Physics
	{
	public:
		Physics() = default;
		~Physics() = default;

		/// @brief Initializes the Jolt job and physics systems
		/// @param p_maxBodies Maximum number of bodies to support.
		/// @param p_numBodyMutexes Number of body mutexes to use. Should be a power of 2 in the range [1, 64], use 0 to auto detect.
		/// @param p_maxBodyPairs Maximum amount of body pairs to process (anything else will fall through the world), this number should generally be much higher than the max amount of contact points as there will be lots of bodies close that are not actually touching.
		/// @param p_maxContactConstraints Maximum amount of contact constraints to process (anything else will fall through the world).
		void Initialize(const std::uint32_t p_maxBodies = 1024, const std::uint32_t p_numBodyMutexes = 0, 
			const std::uint32_t p_maxBodyPairs = 1024, const std::uint32_t p_maxContactConstraints = 1024);

		/// @brief Set the gravity
		/// @param p_gravityVector The gravity acceleration vector
		void SetGravity(const JPH::Vec3 p_gravityVector);
		/// @brief Set the gravity
		/// @param p_gravityVector The gravity acceleration vector
		void SetGravity(const glm::vec3 p_gravityVector);

	private:
		std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
		JPH::JobSystemThreadPool m_jobSystem;
		JPH::PhysicsSystem m_physicsSystem;
		BPLayerInterfaceImpl m_broadPhaseLayerInterface;
		ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadphaseLayerFilter;
		ObjectLayerPairFilterImpl m_objectVsObjectLayerFilter;

		JPH::BodyInterface *m_bodyInterface;

	};
}