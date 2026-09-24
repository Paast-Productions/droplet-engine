#pragma once
#include "resource/types/TextureResource.hpp"

namespace Droplet {
    void TextureResource::SetFormat(TextureFormat p_format, int p_bytesPerPixel)
    {
        m_format = p_format; m_bytesPerPixel = p_bytesPerPixel;
    }

    void TextureResource::SetMipLevels(int p_mipLevels)
    {
        m_mipLevels = p_mipLevels;
    }

    void TextureResource::SetPixelData(const std::vector<std::byte> &p_pixelData)
    {
        m_pixelData = p_pixelData;
    }

    void TextureResource::SetPixelData(const void *p_data, size_t p_size)
    {
        const auto *bytes = static_cast<const std::byte *>(p_data);
        m_pixelData.assign(bytes, bytes + p_size);
    }

    TextureResource::TextureFormat TextureResource::GetFormat() const
    {
        return m_format;
    }

    int TextureResource::GetBytesPerPixel() const
    {
        return m_bytesPerPixel;
    }

    int TextureResource::GetMipLevels() const
    {
        return m_mipLevels;
    }

    const std::vector<std::byte> & TextureResource::GetPixelData() const
    {
        return m_pixelData;
    }
}
