#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#include <Graphics/VK/ImageView.hpp>
#include <optional>

namespace Droplet::Graphics::VK
{
	class DepthBuffer
	{
	public:
		/// @brief Deleted default constructor
		DepthBuffer() = delete;
		/// @brief Default destructor
		~DepthBuffer() = default;

		/// @brief Depth buffer constructor
		/// @param p_device reference to the Vulkan device
		/// @param p_physicalDevice reference to the hardware device
		/// @param p_swapchainExtent reference to the swapchain extent
		DepthBuffer(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::Extent2D &p_swapchainExtent);

		/// @brief Image getter
		/// @return pointer to the image
		inline const vk::raii::Image *GetImage()
		{
			return m_imageView.value().GetImage();
		}

		/// @brief ImageView getter
		/// @return pointer to the imageview
		inline const vk::raii::ImageView *GetView()
		{
			return m_imageView.value().GetView();
		}

		/// @brief DeviceMemory getter
		/// @return pointer to the device memory
		inline const vk::raii::DeviceMemory *GetMemory()
		{
			return m_imageView.value().GetMemory();
		}
	private:

		/// @brief Finds what memory formats are supported by the hardware
		/// @param p_physicalDevice reference to the hardware device
		/// @param candidates list of format candidates to choose from
		/// @param tiling Image tiling flags
		/// @param features Format feature flags
		/// @return the supported format to be used
		vk::Format findSupportedFormat(const vk::raii::PhysicalDevice &p_physicalDevice, 
													const std::vector<vk::Format> &candidates, 
													vk::ImageTiling tiling, 
													vk::FormatFeatureFlags features);

		std::optional<ImageView> m_imageView;
	};
}