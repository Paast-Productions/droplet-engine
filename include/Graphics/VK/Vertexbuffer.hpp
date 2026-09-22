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
	// Vertex structure with position, normal, color, and texture coordinates
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			return { .binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex };
		}

		static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions()
		{
			return { {{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
					 {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
					 {.location = 2, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, normal)},
					 {.location = 3, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}} };
		}
	};

	class VertexBuffer
	{
	public:
		VertexBuffer() = delete;
		~VertexBuffer() = default;

		VertexBuffer(vk::raii::Device const &p_device,
			vk::raii::PhysicalDevice const &p_physDevice,
			vk::raii::CommandPool const &p_commandPool,
			vk::raii::Queue const &p_queue,
			const std::vector<Vertex> &p_vertices);


		const vk::raii::Buffer *GetVertexBuffer();
		const vk::raii::DeviceMemory *GetDeviceMemory();
	private:
		vk::raii::Buffer		m_vertexBuffer = nullptr;
		vk::raii::DeviceMemory	m_bufferMemory = nullptr;
	};

	//I'm worried this may be illegal
	inline const vk::raii::Buffer *VertexBuffer::GetVertexBuffer()
	{
		return &m_vertexBuffer;
	}

	inline const vk::raii::DeviceMemory *VertexBuffer::GetDeviceMemory()
	{
		return &m_bufferMemory;
	}
}
