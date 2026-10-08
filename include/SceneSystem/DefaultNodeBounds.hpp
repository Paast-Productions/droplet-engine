#include <glm/glm.hpp>
#include <SceneSystem/NodeBounds.hpp>
#include <math/bounds/Ray.hpp>
#include <math/bounds/Plane.hpp>
#include <math/bounds/AABB.hpp>
#include <math/bounds/OBB.hpp>
#include <math/bounds/Sphere.hpp>
#include <math/bounds/Frustum.hpp>
#include <math/bounds/Intersection.hpp>

namespace Droplet::Scene
{
    /// @brief The default node bounds containing an AABB
    class DefaultNodeBounds : public NodeBounds
    {
    public:
        DefaultNodeBounds(glm::vec3 p_pos, glm::vec3 p_ext) : m_aabb(p_pos, p_ext) {}
        ~DefaultNodeBounds() = default;

        /// @brief Determines whether the default node bounds intersects with a plane.
        /// @param p_plane The plane to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Plane &p_plane) const override
        {
            return Droplet::Math::Intersects(m_aabb, p_plane);
        }

        /// @brief Determines whether the default node bounds intersects with an AABB.
        /// @param p_aabb The AABB to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::AABB &p_aabb) const override
        {
            return Droplet::Math::Intersects(p_aabb, m_aabb);
        }

        /// @brief Determines whether the default node bounds intersects with an OBB.
        /// @param p_obb The OBB to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::OBB &p_obb) const override
        {
            return Droplet::Math::Intersects(p_obb, m_aabb);
        }

        /// @brief Determines whether the default node bounds intersects with a sphere.
        /// @param p_sphere The sphere to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Sphere &p_sphere) const override
        {
            return Droplet::Math::Intersects(p_sphere, m_aabb);
        }

        /// @brief Determines whether the default node bounds intersects with a frustum.
        /// @param p_frustum The frustum to test against.
	    /// @return An IntersectType value indicating whether the volumes intersect, or if this fully contains the other.
        [[nodiscard]] Droplet::Math::IntersectType Intersect(const Droplet::Math::Frustum &p_frustum) const override
        {
            return Droplet::Math::Intersects(p_frustum, m_aabb);
        }

        /// @brief Performs a raycast against the default node bounds and returns information about the hit.
        /// @param p_ray The ray to cast.
	    /// @return A RayHit struct containing information about the hit, including the hit point, normal, distance, and whether a hit occurred.
        [[nodiscard]] Droplet::Math::RayHit Raycast(const Droplet::Math::Ray &p_ray) const override
        {
            return Droplet::Math::Raycast(p_ray, m_aabb);
        }

    private:
        Droplet::Math::AABB m_aabb;
    };
}