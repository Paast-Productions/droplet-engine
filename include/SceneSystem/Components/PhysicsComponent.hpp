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
		PhysicsComponent() = default;
		~PhysicsComponent();

		/// @brief Initializes the PhysicsComponent
		/// @param p_physics The Physics instance of the Scene where this component exists
		/// @param p_transform The transform of the Node this component is added to
		void Initialize(Physics *p_physics, Transform *p_transform);
		
		/// @brief Gets the type name of the component.
		/// @return The type name of the component.
		std::string_view GetTypeName() const override { return "PhysicsComponent"; }
		
		/// @brief Set a box collider
		/// @param p_halfExtents Vector of the half extents of the box
		void SetBoxCollider(const JPH::Vec3 p_halfExtents, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));
		/// @brief Set a box collider
		/// @param p_halfExtents Vector of the half extents of the box
		void SetBoxCollider(const glm::vec3 p_halfExtents, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));

		/// @brief Set a sphere collider
		/// @param p_radius Radius of the sphere
		void SetSphereCollider(const float p_radius, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));

		/// @brief Set a capsule collider
		/// @param p_HalfHeightOfCylinder Half height of the cylinder part of the capsule
		/// @param p_radius Radius of the capsule
		void SetCapsuleCollider(const float p_HalfHeightOfCylinder, const float p_radius, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));

		/// @brief Set a mesh collider
		/// @param p_mesh The mesh resource to make a collider of
		void SetMeshCollider(const Droplet::MeshResource *p_mesh, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));

		/// @brief Set the motion type of a physics body
		/// @param p_motionType The motion type to be set
		/// 
		/// A motion type defines how a body is allowed to move and interact with the physics simulation.
		/// Motion types:
		/// @li Static, Non movable
		/// @li Kinematic, Movable using velocities only, does not respond to forces
		/// @li Dynamic, Responds to forces as a normal physics object
		void SetMotionType(const JPH::EMotionType p_motionType);

		/// @brief Set what the physics body can collide with
		/// @param p_objectLayer The object layer to be set
		void SetObjectLayer(const Layers p_objectLayer);
		
		void RemoveCollider();

	protected:
		/// @brief Starts the component.
		void StartImpl() override;
		
		/// @brief Updates the component.
		void UpdateImpl() override;

		/// @brief Late updates the physics component.
		void LateUpdateImpl() override;
		
		void RenderUIImpl() override;

		nlohmann::json SerializeImpl() override;
		void DeserializeImpl(nlohmann::json p_compJson) override;


	private:
		/// @brief Helper function for creating collider body
		/// @param p_shapeSettings The shape settings for the collider body
		/// @throw std::runtime_error Mesh collider could not be created
		void CreateCollider(JPH::ShapeSettings &p_shapeSettings, const JPH::Vec3 &p_offset = JPH::Vec3(0.0f, 0.0f, 0.0f));

		/// @brief Sends transform changes from the game engine to Jolt
		void PushTransformToPhysics();

		/// @brief Applies the simulated Jolt transform to the Node Transform
		void PullTransformFromPhysics();
		
		enum class ShapeType : std::uint8_t
		{
			None,
			Box,
			Sphere,
			Capsule,
			Mesh
		}m_shapeType = ShapeType::None;
		JPH::EMotionType m_motion = JPH::EMotionType::Static;
		Layers m_layer = NON_MOVING;

		Physics *m_physics;
		Transform *m_transform;

		JPH::BodyID m_bodyID;
		JPH::ShapeRefC m_shape{};

	};
}