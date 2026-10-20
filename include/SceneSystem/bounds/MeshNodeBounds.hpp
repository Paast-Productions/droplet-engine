#pragma once

#include <glm/glm.hpp>
#include <SceneSystem/bounds/NodeBounds.hpp>
#include <math/bounds/Intersection.hpp>
#include <memory>

namespace Droplet::Scene
{
	// Forward declaration
	class MeshComponent;

	/// @brief Represents the bounds of a MeshComponent.
	class MeshNodeBounds : public NodeBounds, public std::enable_shared_from_this<MeshNodeBounds>
    {
    public:
		MeshNodeBounds() = delete;
        ~MeshNodeBounds() = default;

        MeshNodeBounds(MeshComponent *const p_meshComponent) : m_meshComponent(p_meshComponent) {}

        /// @brief Gets the axis-aligned bounding box (AABB) representation of the node bounds in local-space.
        /// @return The AABB of the node bounds in local-space.
        [[nodiscard]] Math::AABB GetAABB() const override;

        /// @brief Returns a transformed OBB of the node bounds using the provided world matrix.
        /// @param p_worldMat The world matrix to transform the OBB.
        /// @return A transformed OBB representing the node bounds in world space.
        [[nodiscard]] Math::OBB GetTransformedOBB(const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with a plane.
        /// @param p_plane The plane to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Plane &p_plane, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with an AABB.
        /// @param p_aabb The AABB to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::AABB &p_aabb, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with an OBB.
        /// @param p_obb The OBB to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::OBB &p_obb, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with a sphere.
        /// @param p_sphere The sphere to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with a frustum.
        /// @param p_frustum The frustum to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Performs a raycast against the node bounds and returns information about the hit.
        /// @param p_ray The ray to cast.
        /// @param p_worldMat The world matrix of the node.
        /// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
        [[nodiscard]] Math::RayHit Raycast(const Math::Ray &p_ray, const glm::mat4x4 &p_worldMat) const override;

    private:
		MeshComponent *const m_meshComponent = nullptr;
    };
}