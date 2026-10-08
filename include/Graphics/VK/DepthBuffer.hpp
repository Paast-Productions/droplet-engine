#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Graphics/VK/Image.hpp>
#include <Graphics/VK/ImageView.hpp>

namespace Droplet::Graphics::VK
{
	class DepthBuffer
	{
	public:
		DepthBuffer() = delete;
		
		/// @brief DepthBuffer nullptr constructor
		/// @param p_nullptr nullptr
		DepthBuffer(nullptr_t p_nullptr)
		{
			m_image = { p_nullptr };
			m_imageView = { p_nullptr };
		}

		/// @brief Depth buffer constructor
		/// @param p_allocator VMA RAII Allocator
		/// @param p_device reference to the Vulkan device
		/// @param p_physicalDevice reference to the hardware device
		/// @param p_swapchainExtent reference to the swapchain extent
		DepthBuffer(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::Extent2D &p_swapchainExtent);
		
		DepthBuffer(const DepthBuffer &p_other) = delete;
		DepthBuffer &operator=(const DepthBuffer &p_other) = delete;
		
		DepthBuffer(DepthBuffer &&p_other) noexcept
		{
			m_image = std::move(p_other.m_image);
			m_imageView = std::move(p_other.m_imageView);
		}
		
		DepthBuffer &operator=(DepthBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			m_image = std::move(p_other.m_image);
			m_imageView = std::move(p_other.m_imageView);
			
			return *this;
		}
		
		~DepthBuffer() = default;
		
		bool operator==(const DepthBuffer &p_other) const
		{
			return (m_image == p_other.m_image &&
					m_imageView == p_other.m_imageView);
		}
		
		/// @brief Image getter
		/// @returns The depth buffers Vulkan Image
		[[nodiscard]] const vma::raii::Image &GetImage() const { return m_image.Get(); }

		/// @brief ImageView getter
		/// @returns The depth buffers Vulkan Image View
		[[nodiscard]] const vk::raii::ImageView &GetView() const {return m_imageView.Get(); }

	private:

		/// @brief Finds what memory formats are supported by the hardware
		/// @param p_physicalDevice reference to the hardware device
		/// @param p_candidates list of format candidates to choose from
		/// @param p_tiling Image tiling flags
		/// @param p_features Format feature flags
		/// @return the supported format to be used
		[[nodiscard]] vk::Format FindSupportedFormat(const vk::raii::PhysicalDevice &p_physicalDevice,
													const std::vector<vk::Format> &p_candidates, 
													vk::ImageTiling p_tiling,
													vk::FormatFeatureFlags p_features);

		Image m_image { nullptr };
		ImageView m_imageView { nullptr };
	};
}