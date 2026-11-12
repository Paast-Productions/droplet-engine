#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Graphics/VK/CommandPool.hpp>
#include <Graphics/VK/Context.hpp>
#include <Graphics/VK/ImageView.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Image class for storing textures/image buffers to be bound to shader stages
	class Image : public Droplet::IDynamicResource
	{
	public:
		Image()
		{
			m_image = nullptr;
		}
		
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
			const vk::raii::CommandBuffer &p_commandBuffer,
			const std::byte *p_pixels,
			std::size_t p_size,
			std::uint32_t p_width,
			std::uint32_t p_height,
			vk::Format p_format,
			vk::ImageTiling p_tiling,
			vk::ImageUsageFlags p_usage,
			vk::MemoryPropertyFlags p_properties,
			vk::PipelineStageFlagBits2 p_dstStage,
			vk::AccessFlagBits2 p_dstAccess,
			vk::ImageLayout p_newLayout);
		
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
		
		// What is this, and why?
		Image(vma::raii::Image &&p_image) { std::swap(m_image, p_image); }
		
		Image(const Image &p_other) = delete;
		Image &operator=(const Image &p_other) = delete;
		
		Image(Image &&p_other) noexcept
		{
			std::swap(m_image, p_other.m_image);
		}
		
		Image &operator=(Image &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_image, p_other.m_image); 
			
			return *this;
		}

		~Image() final = default;
		
		bool operator==(const Image &p_other) const
		{
			return m_image == p_other.m_image;
		}
		
		void TransitionLayout(
			const vk::raii::CommandBuffer &p_commandBuffer,
			vk::ImageLayout p_newLayout, 
			vk::PipelineStageFlagBits2 p_srcStage,
			vk::PipelineStageFlagBits2 p_dstStage,
			vk::AccessFlagBits2 p_srcAccess,
			vk::AccessFlagBits2 p_dstAccess);
		
		/// @brief Image getter
		/// @return pointer to the image
		[[nodiscard]] const vma::raii::Image &Get() const { return m_image; }
		
		[[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }
		
		[[nodiscard]] vk::Format GetFormat() const { return m_format; }

	private:
		vma::raii::Image m_image { nullptr };
		vk::Extent2D m_extent { 854, 480 };
		vk::Format m_format {};
		vk::ImageLayout m_currentLayout {};
		
	};
}
