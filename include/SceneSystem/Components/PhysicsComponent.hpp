#pragma once

#include <SceneSystem/Component.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <glm/glm.hpp>
#include <Resource/Types/MeshResource.hpp>
#include <SceneSystem/Physics/Physics.hpp>
#include <Transform.hpp>

namespace Droplet::Scene
{
	/// @brief A physics component for some shape
	class PhysicsComponent : public Component
	{
	public:
		/// @brief Constructs the PhysicsComponent
		/// @param p_physics The Physics instance of the Scene where this component exists
		/// @param p_transform The transform of the Node this component is added to
		PhysicsComponent() = default;
		~PhysicsComponent() = default;

		void Initialize(Physics *p_physics, Transform *p_transform);

		/// @brief Gets the type name of the component.
		/// @return The type name of the component.
		std::string_view GetTypeName() const override { return "PhysicsComponent"; }

		/// @brief Updates the component.
		/// @param p_deltaTime Time elapsed since the previous update, in seconds.
		void Update([[maybe_unused]] float p_deltaTime) override;

		/// @brief Late updates the physics component.
        /// @param p_deltaTime Time elapsed since the previous update, in seconds.
		void LateUpdate([[maybe_unused]] float p_deltaTime) override;

		/// @brief Set a box collider
		/// @param p_halfExtents Vector of the half extents of the box
		void SetBoxCollider(const JPH::Vec3 p_halfExtents);
		/// @brief Set a box collider
		/// @param p_halfExtents Vector of the half extents of the box
		void SetBoxCollider(const glm::vec3 p_halfExtents);

		/// @brief Set a sphere collider
		/// @param p_radius Radius of the sphere
		void SetSphereCollider(const float p_radius);

		/// @brief Set a capsule collider
		/// @param p_HalfHeightOfCylinder Half height of the cylinder part of the capsule
		/// @param p_radius Radius of the capsule
		void SetCapsuleCollider(const float p_HalfHeightOfCylinder, const float p_radius);

		/// @brief Set a mesh collider
		/// @param p_mesh The mesh resource to make a collider of
		void SetMeshCollider(const Droplet::MeshResource *p_mesh);

		/// @brief Set the motion type of a physics body
		/// @param p_motionType The motion type to be set
		void SetMotionType(const JPH::EMotionType p_motionType);

		/// @brief Set what the physics body can collide with
		/// @param p_objectLayer The object layer to be set
		void SetObjectLayer(const Layers p_objectLayer);

	private:
		/// @brief Helper function for creating collider body
		/// @param p_shapeSettings The shape settings for the collider body
		/// @throw std::runtime_error Mesh collider could not be created
		void CreateCollider(JPH::ShapeSettings &p_shapeSettings);

		/// @brief Sends transform changes from the game engine to Jolt
		void PushTransformToPhysics();

		/// @brief Applies the simulated Jolt transform to the Node Transform
		void PullTransformFromPhysics();

		bool m_hasShape = false;
		Physics *m_physics;
		Transform *m_transform;

		JPH::BodyID m_bodyID;
		JPH::ShapeRefC m_shape{};

	};
}