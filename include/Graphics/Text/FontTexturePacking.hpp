#pragma once

#include <cstdint>
#include <vector>
#include <glm/vec4.hpp>

namespace Droplet { class FontResource; }

namespace Droplet::Graphics::Text
{
    struct PackedFontTexture
    {
        uint32_t width;
        uint32_t height;
        std::vector<glm::vec4> texels;
    };
    
    [[nodiscard]] PackedFontTexture PackFontTexture(const FontResource& p_font);
}
