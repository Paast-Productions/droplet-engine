#pragma once

#include <glm/glm.hpp>
#include <math/bounds/Ray.hpp>
#include <math/bounds/Plane.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/OBB.hpp>
#include <math/bounds/Sphere.hpp>
#include <math/bounds/Frustum.hpp>
#include <math/bounds/Intersection.hpp>

namespace Droplet::Scene
{
    class NodeBounds
    {
    public:
        virtual ~NodeBounds() = default;

        /// @brief Determines whether the node bounds intersects with a plane.
        /// @param p_plane The plane to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Plane &p_plane, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Determines whether the node bounds intersects with an AABB.
        /// @param p_aabb The AABB to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::AABB &p_aabb, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Determines whether the node bounds intersects with an OBB.
        /// @param p_obb The OBB to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::OBB &p_obb, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Determines whether the node bounds intersects with a sphere.
        /// @param p_sphere The sphere to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Determines whether the node bounds intersects with a frustum.
        /// @param p_frustum The frustum to test against.
        /// @param p_worldMat The world matrix of the node.
        /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Performs a raycast against the node bounds and returns information about the hit.
        /// @param p_ray The ray to cast.
        /// @param p_worldMat The world matrix of the node.
        /// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
        virtual Droplet::Math::RayHit Raycast(const Droplet::Math::Ray &p_ray, const glm::mat4x4 &p_worldMat) const = 0;

        /// @brief Gets the axis-aligned bounding box (AABB) representation of the node bounds in local-space.
        /// @return The AABB of the node bounds in local-space.
        virtual Droplet::Math::AABB GetAABB() const = 0;
    };
}