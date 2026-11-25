#pragma once

#include <cstdint>
#include <stdexcept>

#include <vulkan/vulkan_raii.hpp>

#include <resource/types/TextureResource.hpp>

namespace Droplet::Graphics::Vulkan::TextureFormatUtils
{
	/// @brief How texels of a format are laid out in memory.
	struct BlockLayout
	{
		std::uint32_t blockWidth;	// Texels per block horizontally (1 for uncompressed formats)
		std::uint32_t blockHeight;	// Texels per block vertically (1 for uncompressed formats)
		std::uint32_t blockBytes;	// Bytes per block
	};

	/// @brief Converts a texture resource format to its Vulkan equivalent.
	/// @return The matching Vulkan format, or eUndefined if the format is not supported for rendering.
	[[nodiscard]] inline vk::Format ToVkFormat(TextureResource::TextureFormat p_format)
	{
		switch (p_format)
		{
		case TextureResource::TextureFormat::R8_Unorm:      return vk::Format::eR8Unorm;
		case TextureResource::TextureFormat::RGB8_Unorm:    return vk::Format::eR8G8B8Unorm;
		case TextureResource::TextureFormat::RGB8_Srgb:     return vk::Format::eR8G8B8Srgb;
		case TextureResource::TextureFormat::RGBA8_Unorm:   return vk::Format::eR8G8B8A8Unorm;
		case TextureResource::TextureFormat::RGBA8_Srgb:    return vk::Format::eR8G8B8A8Srgb;
		case TextureResource::TextureFormat::BC1:           return vk::Format::eBc1RgbaUnormBlock;
		case TextureResource::TextureFormat::BC1_Srgb:      return vk::Format::eBc1RgbaSrgbBlock;
		case TextureResource::TextureFormat::BC2:           return vk::Format::eBc2UnormBlock;
		case TextureResource::TextureFormat::BC3:           return vk::Format::eBc3UnormBlock;
		case TextureResource::TextureFormat::BC3_Srgb:      return vk::Format::eBc3SrgbBlock;
		case TextureResource::TextureFormat::BC4:           return vk::Format::eBc4UnormBlock;
		case TextureResource::TextureFormat::BC5:           return vk::Format::eBc5UnormBlock;
		case TextureResource::TextureFormat::BC6H:          return vk::Format::eBc6HUfloatBlock;
		case TextureResource::TextureFormat::BC7:           return vk::Format::eBc7UnormBlock;
		case TextureResource::TextureFormat::BC7_Srgb:      return vk::Format::eBc7SrgbBlock;
		default:                                            return vk::Format::eUndefined;
		}
	}

	/// @brief Gets the block layout of a Vulkan format.
	/// @throws std::runtime_error If the format's layout is not known.
	[[nodiscard]] inline BlockLayout GetBlockLayout(vk::Format p_format)
	{
		switch (p_format)
		{
		case vk::Format::eR8Unorm:
			return { 1, 1, 1 };
		case vk::Format::eR8G8B8Unorm:
		case vk::Format::eR8G8B8Srgb:
			return { 1, 1, 3 };
		case vk::Format::eR8G8B8A8Unorm:
		case vk::Format::eR8G8B8A8Srgb:
			return { 1, 1, 4 };
		case vk::Format::eBc1RgbaUnormBlock:
		case vk::Format::eBc1RgbaSrgbBlock:
		case vk::Format::eBc4UnormBlock:
			return { 4, 4, 8 };
		case vk::Format::eBc2UnormBlock:
		case vk::Format::eBc3UnormBlock:
		case vk::Format::eBc3SrgbBlock:
		case vk::Format::eBc5UnormBlock:
		case vk::Format::eBc6HUfloatBlock:
		case vk::Format::eBc7UnormBlock:
		case vk::Format::eBc7SrgbBlock:
			return { 4, 4, 16 };
		default:
			throw std::runtime_error("Unsupported texture format");
		}
	}
}
