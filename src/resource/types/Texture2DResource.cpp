#include "resource/types/Texture2DResource.hpp"
#include <tracy/public/tracy/Tracy.hpp>

namespace Droplet
{
    std::unique_ptr<Texture2DResource> Texture2DResource::CreateFallback()
    {
        ZoneScoped;

        auto fallback = std::make_unique<Texture2DResource>();
        
        fallback->SetDimensions(2, 2);
        fallback->SetFormat(TextureFormat::RGBA8_Unorm, 4);
        fallback->SetMipLevels(1);
        
        std::uint8_t pixels[16] = {
            255, 0, 255, 255, // Magenta
            0, 0, 0, 255,     // Black
            0, 0, 0, 255,     // Black
            255, 0, 255, 255  // Magenta
        };
        fallback->SetPixelData(pixels, sizeof(pixels));
        
        return fallback;
    }
}
