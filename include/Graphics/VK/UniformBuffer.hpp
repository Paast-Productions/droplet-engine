#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <glm/glm.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Generic Uniform Buffer layout
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	/// @brief Uniform Buffer Class
	class UniformBuffer
	{
	public:
		UniformBuffer() = delete;

		/// @brief Uniform Buffer constructor
		/// @param p_device RAII pointer reference to the Device
		/// @param p_physDevice RAII pointer reference to the hardware Device
		UniformBuffer(vk::raii::Device const &p_device,
			vk::raii::PhysicalDevice const &p_physDevice);
		~UniformBuffer() = default;

		/// @brief Updates the buffer
		/// @param p_swapchainExtent The extent of the swapchain
		void UpdateBuffer(const vk::Extent2D &p_swapchainExtent) const;

		/// @brief Buffer Getter
		/// @return RAII pointer to the uniform buffer
		const vk::raii::Buffer *GetBuffer();

		/// @brief Device Memory Getter
		/// @return RAII pointer to the device memory of the buffer
		const vk::raii::DeviceMemory *GetDeviceMemory();

		/// @brief Buffer Mapping Getter
		/// @return void pointer to the mapped buffer
		const void *GetMappedBuffer();
	private:

		vk::raii::Buffer	   m_uniformBuffer = nullptr;
		vk::raii::DeviceMemory m_deviceMemory = nullptr;
		void *m_mappedBuffer; // GPU Memory Address
	};

	inline const vk::raii::Buffer *UniformBuffer::GetBuffer()
	{
		return &m_uniformBuffer;
	}

	inline const vk::raii::DeviceMemory *UniformBuffer::GetDeviceMemory()
	{
		return &m_deviceMemory;
	}

	inline const void *UniformBuffer::GetMappedBuffer()
	{
		return &m_mappedBuffer;
	}
}

