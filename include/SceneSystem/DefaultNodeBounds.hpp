#pragma once

#include <glm/glm.hpp>
#include <SceneSystem/bounds/NodeBounds.hpp>
#include <math/bounds/Intersection.hpp>
#include <math/bounds/Conversion.hpp>

namespace Droplet::Scene
{
    /// @brief The default node bounds containing an AABB
    class DefaultNodeBounds : public NodeBounds
    {
    public:
        DefaultNodeBounds(glm::vec3 p_pos, glm::vec3 p_ext) : m_aabb(p_pos, p_ext) {}
        ~DefaultNodeBounds() = default;

        /// @brief Gets the axis-aligned bounding box (AABB) representation of the node bounds in local-space.
        /// @return The AABB of the node bounds in local-space.
        [[nodiscard]] Math::AABB GetAABB() const override
        {
            return m_aabb;
        }

		/// @brief Returns a transformed OBB of the default node bounds using the provided world matrix.
		/// @param p_worldMat The world matrix to transform the OBB.
		/// @return A transformed OBB representing the default node bounds in world space.
        [[nodiscard]] Math::OBB GetTransformedOBB(const glm::mat4x4 &p_worldMat) const
        {
            return Math::OBB::Transform(Math::AABBToOBB(m_aabb), p_worldMat);
        }

        /// @brief Determines whether the default node bounds intersects with a plane.
        /// @param p_plane The plane to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Plane &p_plane, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Intersects(GetTransformedOBB(p_worldMat), p_plane);
        }

        /// @brief Determines whether the default node bounds intersects with an AABB.
        /// @param p_aabb The AABB to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::AABB &p_aabb, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Intersects(p_aabb, GetTransformedOBB(p_worldMat));
        }

        /// @brief Determines whether the default node bounds intersects with an OBB.
        /// @param p_obb The OBB to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::OBB &p_obb, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Intersects(p_obb, GetTransformedOBB(p_worldMat));
        }

        /// @brief Determines whether the default node bounds intersects with a sphere.
        /// @param p_sphere The sphere to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Intersects(p_sphere, GetTransformedOBB(p_worldMat));
        }

        /// @brief Determines whether the default node bounds intersects with a frustum.
        /// @param p_frustum The frustum to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Math::IntersectType Intersect(const Math::Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Intersects(p_frustum, GetTransformedOBB(p_worldMat));
        }

        /// @brief Performs a raycast against the default node bounds and returns information about the hit.
        /// @param p_ray The ray to cast.
	    /// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
        [[nodiscard]] Math::RayHit Raycast(const Math::Ray &p_ray, const glm::mat4x4 &p_worldMat) const override
        {
            return Math::Raycast(p_ray, GetTransformedOBB(p_worldMat));
        }

    private:
        Math::AABB m_aabb;
    };
}