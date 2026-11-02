#pragma once

#include <vector>

#include "resource/IResource.hpp"

namespace Droplet
{
    /// @brief Abstract class for all texture resources.
	class TextureResource : public IResource
	{
	public:
		enum class TextureFormat
		{
			Unknown,

			// Color formats
			R8_Unorm,
			R8_Snorm,
			R8_Uscaled,
			R8_Sscaled,
			R8_Uint,
			R8_Sint,

			R16_Unorm,
			R16_Snorm,
			R16_Uscaled,
			R16_Sscaled,
			R16_Uint,
			R16_Sint,

			R32_Unorm,
			R32_Snorm,
			R32_Uscaled,
			R32_Sscaled,
			R32_Uint,
			R32_Sint,

			RGB8_Unorm,
			RGB8_Snorm,
			RGB8_Uscaled,
			RGB8_Sscaled,
			RGB8_Uint,
			RGB8_Sint,

			RGB16_Unorm,
			RGB16_Snorm,
			RGB16_Uscaled,
			RGB16_Sscaled,
			RGB16_Uint,
			RGB16_Sint,

			RGB32_Unorm,
			RGB32_Snorm,
			RGB32_Uscaled,
			RGB32_Sscaled,
			RGB32_Uint,
			RGB32_Sint,

			RGBA8_Unorm,
			RGBA8_Snorm,
			RGBA8_Uscaled,
			RGBA8_Sscaled,
			RGBA8_Uint,
			RGBA8_Sint,
			   
			RGBA16_Unorm,
			RGBA16_Snorm,
			RGBA16_Uscaled,
			RGBA16_Sscaled,
			RGBA16_Uint,
			RGBA16_Sint,
			   
			RGBA32_Unorm,
			RGBA32_Snorm,
			RGBA32_Uscaled,
			RGBA32_Sscaled,
			RGBA32_Uint,
			RGBA32_Sint,

			// Depth formats
			D24S8,
			D32,

			// Compressed formats
			BC1,
			BC2,
			BC3,
			BC4,
			BC5,
			BC6H,
			BC7,

			// Add more formats as needed...
		};

		/// @brief Sets the format of the texture data.
		/// @param p_format The format of the texture data.
		/// @param p_bytesPerPixel The number of bytes per pixel in the texture data.
		void SetFormat(TextureFormat p_format, int p_bytesPerPixel);

		/// @brief Sets the number of mipmap levels in the texture.
		/// @param p_mipLevels The number of mipmap levels in the texture.
		void SetMipLevels(int p_mipLevels);

		/// @brief Sets the pixel data for the texture.
		/// @param p_pixelData The pixel data for the texture.
		void SetPixelData(const std::vector<std::byte> &p_pixelData);

		/// @brief Copies the pixel data into the texture resource.
		/// @param p_data Pointer to the raw data.
		/// @param p_size The size of the data.
		void SetPixelData(const void *p_data, size_t p_size);

		/// @brief Gets the format of the texture data.
		/// @return The format of the texture data.
		[[nodiscard]] TextureFormat GetFormat() const;

		/// @brief Gets the number of bytes per pixel in the texture data.
		/// @return The number of bytes per pixel in the texture data.
		[[nodiscard]] int GetBytesPerPixel() const;

		/// @brief Gets the number of mipmap levels in the texture.
		/// @return The number of mipmap levels in the texture.
		[[nodiscard]] int GetMipLevels() const;

		/// @brief Gets the pixel data for the texture.
		/// @return A vector of bytes representing the pixel data for the texture.
		[[nodiscard]] const std::vector<std::byte> &GetPixelData() const;

	private:
		TextureFormat	m_format = TextureFormat::Unknown;	// The format of the texture data
		int				m_mipLevels = 1;					// The number of mipmap levels in the texture
		int				m_bytesPerPixel = 0;				// The number of bytes per pixel in the texture data (invalid for compressed formats)

		std::vector<std::byte> m_pixelData{};				// Pixel data for the texture
	};
}