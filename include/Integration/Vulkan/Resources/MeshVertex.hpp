#pragma once
#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.hpp>

/// Vertex formats as the GPU sees them (Vulkan input descriptions)
namespace Vulkan::Vertex
{
    /// Position / normal / uv. Same layout as Common::MeshVertex.
    struct P3N3UV2
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        static constexpr vk::VertexInputBindingDescription GetBindingDescription()
        {
            return {.binding = 0, .stride = static_cast<std::uint32_t>(sizeof(P3N3UV2)), .inputRate = vk::VertexInputRate::eVertex};
        }

        static constexpr std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescription()
        {
            return {{
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(P3N3UV2, position)},
                {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(P3N3UV2, normal)},
                {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat,    .offset = offsetof(P3N3UV2, uv)},
            }};
        }
    };
}
