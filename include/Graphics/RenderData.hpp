#pragma once

#include <cstdint>
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
    };
}

