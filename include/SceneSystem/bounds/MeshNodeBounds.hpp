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
    class MeshNodeBounds : public NodeBounds
    {
    public:
		MeshNodeBounds() = delete;
        ~MeshNodeBounds() = default;

        MeshNodeBounds(std::weak_ptr<MeshComponent> p_meshComponent) : m_meshComponent(p_meshComponent) {}

        /// @brief Determines whether the node bounds intersects with a plane.
        /// @param p_plane The plane to test against.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Plane &p_plane, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with an AABB.
        /// @param p_aabb The AABB to test against.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::AABB &p_aabb, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with an OBB.
        /// @param p_obb The OBB to test against.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::OBB &p_obb, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with a sphere.
        /// @param p_sphere The sphere to test against.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Determines whether the node bounds intersects with a frustum.
        /// @param p_frustum The frustum to test against.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const override;

        /// @brief Performs a raycast against the node bounds and returns information about the hit.
        /// @param p_ray The ray to cast.
        /// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
        [[nodiscard]] Droplet::Math::RayHit Raycast(const Droplet::Math::Ray &p_ray, const glm::mat4x4 &p_worldMat) const override;

        [[nodiscard]] Droplet::Math::AABB GetAABB() const override;

    private:
		std::weak_ptr<MeshComponent> m_meshComponent;
    };
}