#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Graphics/VK/CommandPool.hpp>
#include <Graphics/VK/Context.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Image class for storing textures/image buffers to be bound to shader stages
	class Image
	{
	public:
		Image() = delete;
		
		/// @brief Image Nullptr Constructor
		/// @param p_nullptr Nullptr
		Image(nullptr_t p_nullptr)
		{
			m_image = { p_nullptr };
		}
		
		// TODO: <REFACTOR>  
		// EXPLANATION: Should be one singular constructor that can take in all combinations of flags.
		// If you would like a specific combination, create a factory class

		/// @brief Constructor for image based on texture data
		/// @param p_allocator VMA RAII Allocator
		/// @param p_commandPool Vulkan Command Pool
		/// @param p_context Vulkan Context
		/// @param p_pixels unsigned char pointer to the image/texture data
		/// @param p_size size of texture data
		/// @param p_width width of the texture in pixels
		/// @param p_height height of the texture in pixels
		/// @param p_format color/data format of the texture
		/// @param p_tiling Image tiling flag
		/// @param p_usage Image usage flags
		/// @param p_properties Memory property flags
		Image(const vma::raii::Allocator &p_allocator,
			Droplet::Graphics::VK::CommandPool &p_commandPool,
			Droplet::Graphics::VK::Context &p_context,
			const unsigned char *p_pixels,
			const std::size_t p_size,
			std::uint32_t p_width,
			std::uint32_t p_height,
			vk::Format p_format,
			vk::ImageTiling p_tiling,
			vk::ImageUsageFlags p_usage,
			vk::MemoryPropertyFlags p_properties);
		
		/// @brief Constructor for creating an imageview
		/// @param p_allocator VMA RAII Allocator
		/// @param p_width width of the image
		/// @param p_height height of the image
		/// @param p_format data format of the image
		/// @param p_tiling Image tiling flag
		/// @param p_usage Image usage flags
		/// @param p_properties Memory property flags
		Image(const vma::raii::Allocator &p_allocator,
			std::uint32_t p_width,
			std::uint32_t p_height,
			vk::Format p_format,
			vk::ImageTiling p_tiling,
			vk::ImageUsageFlags p_usage,
			vk::MemoryPropertyFlags p_properties);
	
		// TODO: </REFACTOR>
		
		Image(const Image &p_other) = delete;
		Image &operator=(const Image &p_other) = delete;
		
		Image(Image &&p_other) noexcept
		{
			m_image = std::move(p_other.m_image);
		}
		
		Image &operator=(Image &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			m_image = std::move(p_other.m_image); 
			
			return *this;
		}

		~Image() = default;
		
		bool operator==(const Image &p_other) const
		{
			return m_image == p_other.m_image;
		}
		
		/// @brief Image getter
		/// @return pointer to the image
		[[nodiscard]] const vma::raii::Image &Get() const;

	private:
		vma::raii::Image m_image = nullptr;
		
	};

	inline const vma::raii::Image &Image::Get() const
	{
		return m_image;
	}
}