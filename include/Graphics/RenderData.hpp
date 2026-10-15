#pragma once

#include <cstdint>
#include <vector>

namespace Droplet::Graphics
{
    struct ShaderData
    {
        std::uint64_t GUID {0};
        std::vector<std::uint16_t> pushConstants {};
    };
    
    struct RenderData
    {
        ShaderData vsData {};
        ShaderData tcsData {};
        ShaderData tesData {};
        ShaderData gsData {};
        ShaderData fsData {};
    };
}

