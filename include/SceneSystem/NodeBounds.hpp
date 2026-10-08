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

        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Plane &p_plane, const glm::mat4x4 &p_worldMat) const = 0;
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::AABB &p_aabb, const glm::mat4x4 &p_worldMat) const = 0;
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::OBB &p_obb, const glm::mat4x4 &p_worldMat) const = 0;
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Sphere &p_sphere, const glm::mat4x4 &p_worldMat) const = 0;
        virtual Droplet::Math::IntersectType Intersect(const Droplet::Math::Frustum &p_frustum, const glm::mat4x4 &p_worldMat) const = 0;

        virtual Droplet::Math::RayHit Raycast(const Droplet::Math::Ray &p_ray, const glm::mat4x4 &p_worldMat) const = 0;
    };
}