#pragma once 
#include "resource/types/TextureResource.hpp"

#include <cstdint>

namespace Droplet
{
    /// @brief Class for 3D texture resources.
    class Texture3DResource : public TextureResource
    {
    public:
        /// @brief Sets the width, height, and depth of the texture.
        /// @param p_width The width of the texture.
        /// @param p_height The height of the texture.
        /// @param p_depth The depth of the texture.
        void SetDimensions(std::uint32_t p_width, std::uint32_t p_height, std::uint32_t p_depth)
        {
            m_width = p_width;
            m_height = p_height;
            m_depth = p_depth;
        }

        /// @brief Gets the width of the texture.
        /// @return The width of the texture.
        [[nodiscard]] std::uint32_t GetWidth() const { return m_width; }

        /// @brief Gets the height of the texture.
        /// @return The height of the texture.
        [[nodiscard]] std::uint32_t GetHeight() const { return m_height; }

        /// @brief Gets the depth of the texture.
        /// @return The depth of the texture.
        [[nodiscard]] std::uint32_t GetDepth() const { return m_depth; }

    private:
        std::uint32_t m_width = 0;		// The width of the texture
        std::uint32_t m_height = 0;		// The height of the texture
        std::uint32_t m_depth = 0;		// The depth of the texture
    };
}
