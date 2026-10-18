#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

namespace Droplet::Graphics
{
    struct ShaderData
    {
        std::uint64_t GUID {0};
        std::vector<std::uint32_t> pushConstants {};
    };

    struct MeshData
    {
        //Extra mesh rendering options could possibly be added here

        std::uint64_t GUID {0};
    };

    struct TextureData
    {
        //Texture sampling options could be added here
        std::uint64_t GUID {0};

        std::uint64_t samplerIndex;
    };
    
    struct RenderData
    {
        ShaderData vsData {};
        ShaderData tcsData {};
        ShaderData tesData {};
        ShaderData gsData {};
        ShaderData fsData {};

        MeshData meshData{};

        TextureData texData{};

        glm::mat4 transform;
    };

    namespace VK
    {
        struct WorldBuffer
        {
            glm::mat4 matrix{};
        };

        struct ViewProjectionBuffer
        {
            glm::mat4 view{};
            glm::mat4 proj{};
        };
    }

}

