#pragma once

#include <glm/glm.hpp>

namespace Droplet::Scene
{
    struct BoundingBox
    {
        glm::vec3 min{-0.5f, -0.5f, -0.5f};
        glm::vec3 max{0.5f, 0.5f, 0.5f};

        [[nodiscard]] glm::vec3 GetSize() const
        {
            return max - min;
        }

        [[nodiscard]] glm::vec3 GetCenter() const
        {
            return (min + max) * 0.5f;
        }
    };
}