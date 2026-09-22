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
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class UniformBuffer
	{
	public:
		UniformBuffer() = delete;
		UniformBuffer(vk::raii::Device const &p_device,
			vk::raii::PhysicalDevice const &p_physDevice);
		~UniformBuffer() = default;

		void UpdateBuffer(const vk::Extent2D &p_swapchainExtent);

		const vk::raii::Buffer *GetBuffer();
		const vk::raii::DeviceMemory *GetDeviceMemory();
		const void *GetMappedBuffer();
	private:

		vk::raii::Buffer	   m_uniformBuffer = nullptr;
		vk::raii::DeviceMemory m_deviceMemory = nullptr;
		void *m_mappedBuffer;
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

