#include "Graphics/Text/FontTexturePacking.hpp"

#include "FontResource.hpp"

namespace Droplet::Graphics::Text
{
    PackedFontTexture PackFontTexture(const FontResource& p_font)
    {
	    ZoneScopedN("Text / PackFontTexture")
        
        constexpr size_t width = FontResource::TEXTURE_WIDTH;

        const auto& curves = p_font.GetCurvesTextureData();
        const auto& bands = p_font.GetBandsTextureData();
        
        if (p_font.GetCurveTextureWidth() != width || p_font.GetBandsTextureWidth() != width)
        {
            throw std::runtime_error("Unexpected .slug texture width.");
        }
        
        if (curves.size() != width * p_font.GetCurveTextureHeight() ||
            bands.size() != width * p_font.GetBandsTextureHeight())
        {
            throw std::runtime_error("Invalid .slug texture dimensions.");
        }
        
        const size_t curveRows = p_font.GetCurveTextureHeight();
        const size_t bandRows = p_font.GetBandsTextureHeight();
        
        const size_t height = 1 + curveRows + bandRows;
        
        PackedFontTexture result{
            .width = static_cast<uint32_t>(width),
            .height = static_cast<uint32_t>(height),
            .texels = std::vector<glm::vec4>(width * height, glm::vec4(0.0f))
        };
        
        result.texels[0] = glm::vec4(1.0f, static_cast<float>(1 + curveRows), 0.0f, 0.0f);
        
        std::ranges::copy(
            curves.begin(),
            curves.end(),
            result.texels.begin() + width
        );
        
        const size_t bandStart = (1 + curveRows) * width;
        
        for (size_t i = 0; i < bands.size(); ++i)
        {
            result.texels[bandStart + i] = glm::vec4(
                static_cast<float>(bands[i].x),
                static_cast<float>(bands[i].y),
                0.0f,
                0.0f
            );
        }
        
        return result;
    }
}
