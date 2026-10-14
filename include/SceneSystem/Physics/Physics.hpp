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

		/// @brief A struct contaning values to initialize Physics
		struct Initializer
		{
			std::uint32_t maxBodies = 1024; // Maximum number of bodies to support.
			std::uint32_t numBodyMutexes = 0; // Number of body mutexes to use. Should be a power of 2 in the range [1, 64], use 0 to auto detect.
			std::uint32_t maxBodyPairs = 1024; // Maximum amount of body pairs to process (anything else will fall through the world), this number should generally be much higher than the max amount of contact points as there will be lots of bodies close that are not actually touching.
			std::uint32_t maxContactConstraints = 1024; // Maximum amount of contact constraints to process (anything else will fall through the world).
		};

		Physics() = default;
		/// @brief Constructs physics with custom defined initialize values
		/// @param p_initializer Struct containing custom defined initialize values
		Physics(const Initializer p_initializer);
		~Physics() = default;

		/// @brief Initializes the Jolt job and physics systems
		/// @param p_maxBodies Maximum number of bodies to support.
		/// @param p_numBodyMutexes Number of body mutexes to use. Should be a power of 2 in the range [1, 64], use 0 to auto detect.
		/// @param p_maxBodyPairs Maximum amount of body pairs to process (anything else will fall through the world), this number should generally be much higher than the max amount of contact points as there will be lots of bodies close that are not actually touching.
		/// @param p_maxContactConstraints Maximum amount of contact constraints to process (anything else will fall through the world).
		void Initialize(const std::uint32_t p_maxBodies = 1024, const std::uint32_t p_numBodyMutexes = 0, 
			const std::uint32_t p_maxBodyPairs = 1024, const std::uint32_t p_maxContactConstraints = 1024);
		void Initialize(const Initializer p_initializer);

		/// @brief Updates the physics system
		void FixedUpdate(const float p_fixedDeltaTime);

		/// @brief Updates the physics system
		void Update();

		/// @brief Set the gravity
		/// @param p_gravityVector The gravity acceleration vector
		void SetGravity(const JPH::Vec3 p_gravityVector);
		/// @brief Set the gravity
		/// @param p_gravityVector The gravity acceleration vector
		void SetGravity(const glm::vec3 p_gravityVector);

		/// @brief Getter for the body interface
		/// @return Pointer to the body interface
		[[nodiscard]] JPH::BodyInterface *GetBodyInterface();

	private:
		std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator{};
		JPH::JobSystemThreadPool m_jobSystem{};
		JPH::PhysicsSystem m_physicsSystem{};
		BPLayerInterfaceImpl m_broadPhaseLayerInterface{};
		ObjectVsBroadPhaseLayerFilterImpl m_objectVsBroadphaseLayerFilter{};
		ObjectLayerPairFilterImpl m_objectVsObjectLayerFilter{};

		JPH::BodyInterface *m_bodyInterface{};

	};
}