#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
	/// @brief Descriptor pool class that manages descriptor sets and their layouts for Vulkan resource binding
	class DescriptorPool
	{
	public:
		/// @brief Deleted default constructor
		DescriptorPool() = delete;

		/// @brief Default deconstructor
		~DescriptorPool() = default;

		/// @brief Constructor for the Descriptor Pool Class
		/// @param p_device RAII pointer to the Vulkan device
		/// @param p_maxFramesInFlight highest count of frames in flight to be used in runtime
		DescriptorPool(const vk::raii::Device &p_device, std::uint32_t p_maxFramesInFlight);
		
		/// @brief Getter function for Vulkan Descriptor Pool
		/// @returns Vulkan Descriptor Pool
		[[nodiscard]] const vk::raii::DescriptorPool &Get() const;

	private:
		vk::raii::DescriptorPool m_descriptorPool = nullptr;

	};

	inline vk::raii::DescriptorPool const &DescriptorPool::Get() const
	{
		return m_descriptorPool;
	}
}
