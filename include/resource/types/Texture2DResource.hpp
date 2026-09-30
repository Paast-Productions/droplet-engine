#pragma once
#include "resource/types/TextureResource.hpp"

namespace Droplet {
    /// @brief Class for 2D texture resources.s
	class Texture2DResource : public TextureResource
	{
	public:
		/// @brief Sets the width and height of the texture.
		/// @param p_width The width of the texture.
		/// @param p_height The height of the texture.
		void SetDimensions(std::uint32_t p_width, std::uint32_t p_height)
		{
			m_width = p_width;
			m_height = p_height;
		}

		/// @brief Gets the width of the texture.
		/// @return The width of the texture.
		[[nodiscard]] std::uint32_t GetWidth() const { return m_width; }

		/// @brief Gets the height of the texture.
		/// @return The height of the texture.
		[[nodiscard]] std::uint32_t GetHeight() const { return m_height; }

	private:
		std::uint32_t m_width = 0;		// The width of the texture
		std::uint32_t m_height = 0;		// The height of the texture
	};
}