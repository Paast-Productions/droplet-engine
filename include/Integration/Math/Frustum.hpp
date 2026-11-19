#pragma once
#include <glm/glm.hpp>
#include <array>

namespace Math
{
    /// Six normalized planes (xyz = inward normal, w = distance) 
    /// Inside when dot(n, p) + w >= 0
    struct Frustum
    {
        enum Side { Left, Right, Bottom, Top, Near, Far, Count };

        // a plane is a 4d vector with normalized normal pointing inward
        // a point is inside if 
        std::array<glm::vec4, Count> planes{};

        // helper functions
        static float SignedDistanceTo(glm::vec4 plane, glm::vec3 point) { return glm::dot(glm::vec3(plane), point) + plane.w; }
        float SignedDistance(Side side, glm::vec3 point) const { return SignedDistanceTo(planes[side], point); }

        bool Contains(glm::vec3 point)
        {
            for (glm::vec4& plane : planes)
            {
                if (SignedDistanceTo(plane, point) < 0.0f)
                    return false;
            }
            return true;
        }
    };

    /// Gribb-Hartmann style extraction (thanks google) for a Vulkan-style clip volume (RH, depth 0..1, e.g. glm::perspectiveRH_ZO)
    inline Frustum ExtractFrustumFromViewProj(const glm::mat4& viewProj)
    {
        const auto getRow = [&viewProj](int i)
            {
                return glm::vec4(viewProj[0][i], viewProj[1][i], viewProj[2][i], viewProj[3][i]);
            };
        const glm::vec4 rowX = getRow(0);
        const glm::vec4 rowY = getRow(1);
        const glm::vec4 rowZ = getRow(2);
        const glm::vec4 rowW = getRow(3);

        Frustum frustum;
        frustum.planes[Frustum::Left] = rowW + rowX;
        frustum.planes[Frustum::Right] = rowW - rowX;
        frustum.planes[Frustum::Bottom] = rowW + rowY;
        frustum.planes[Frustum::Top] = rowW - rowY;
        frustum.planes[Frustum::Near] = rowZ; // depth range is 0, 1 not -1, 1
        frustum.planes[Frustum::Far] = rowW - rowZ;
        // frustum.planes[Frustum::Whereveryouare]

        // normalize so dot(plane, point) gives signed distance
        for (glm::vec4& plane : frustum.planes)
            plane /= glm::length(glm::vec3(plane));

        return frustum;
    }

    /// Returns true if a sphere intersects the frustum
    inline bool IntersectsSphere(const Frustum& frustum, const glm::vec3& center, float radius)
    {
        for (const glm::vec4& plane : frustum.planes)
            if (glm::dot(glm::vec3(plane), center) + plane.w < -radius)
                return false;
        return true;
    }
}
