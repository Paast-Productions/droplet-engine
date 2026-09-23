#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vector>

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <glm/glm.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Index Buffer Class
	class IndexBuffer
	{
	public:
		/// @brief Deleted constructor
		IndexBuffer() = delete;

		/// @brief Default destructor
		~IndexBuffer() = default;

		/// @brief Index Buffer constructor
		/// @param p_device RAII pointer to the Vulkan Device
		/// @param p_physDevice RAII pointer to the device hardware
		/// @param p_commandPool RAII pointer to the Command Pool
		/// @param p_queue RAII pointer to the Vulkan Queue
		/// @param p_indices Vector containing index data
		IndexBuffer(vk::raii::Device const &p_device,
			vk::raii::PhysicalDevice const &p_physDevice,
			vk::raii::CommandPool const &p_commandPool,
			vk::raii::Queue const &p_queue,
			const std::vector<uint16_t> &p_indices);


		const vk::raii::Buffer *GetIndexBuffer();
		const vk::raii::DeviceMemory *GetDeviceMemory();
	private:
		vk::raii::Buffer		m_indexBuffer = nullptr;
		vk::raii::DeviceMemory	m_bufferMemory = nullptr;
	};

	inline const vk::raii::Buffer *IndexBuffer::GetIndexBuffer()
	{
		return &m_indexBuffer;
	}

	inline const vk::raii::DeviceMemory *IndexBuffer::GetDeviceMemory()
	{
		return &m_bufferMemory;
	}
}