#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include <array>

namespace Droplet::Sandbox
{
    struct Vertex
    {
        glm::vec2 pos;
        glm::vec3 color;

        static vk::VertexInputBindingDescription GetBindingDescription()
        {
            return {
                .binding = 0, .stride = static_cast<uint32_t>(sizeof(Vertex)), .inputRate = vk::VertexInputRate::eVertex
            };
        }

        static std::array<vk::VertexInputAttributeDescription, 2> GetAttributeDescription()
        {
            return {{
                {.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
                {
                    .location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat,
                    .offset = offsetof(Vertex, color)
                }
            }};
        };
    };
}