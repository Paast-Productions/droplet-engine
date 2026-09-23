#pragma once

#include <vector>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Vertex structure with position, normal, color, and texture coordinates
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec3 normal;
		glm::vec2 texCoord;

		/// @brief Gets vertex binding flags and vertex size
		/// @return Data required for binding the vertex buffer
		static vk::VertexInputBindingDescription GetBindingDescription()
		{
			return { .binding = 0, .stride = sizeof(Vertex), .inputRate = vk::VertexInputRate::eVertex };
		}

		/// @brief Getter of the vertex layout
		/// @return array containing vertex input descriptions
		static std::array<vk::VertexInputAttributeDescription, 4> GetAttributeDescriptions()
		{
			return { {{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
					 {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
					 {.location = 2, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, normal)},
					 {.location = 3, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}} };
		}
	};

	/// @brief Vertex Buffer Class
	class VertexBuffer
	{
	public:
		/// @brief Deleted default constructor
		VertexBuffer() = delete;

		/// @brief Default destructor
		~VertexBuffer() = default;

		/// @brief Constructor for the VertexBuffer class
		/// @param p_device RAII pointer reference to a Vulkan Device
		/// @param p_physDevice RAII pointer reference to the hardware Device
		/// @param p_commandPool RAII pointer reference to the Command Pool
		/// @param p_queue RAII pointer reference to the Vulkan Queue
		/// @param p_vertices vector containing Vertex struct data
		VertexBuffer(vk::raii::Device const &p_device,
			vk::raii::PhysicalDevice const &p_physDevice,
			vk::raii::CommandPool const &p_commandPool,
			vk::raii::Queue const &p_queue,
			const std::vector<Vertex> &p_vertices);

		/// @brief VertexBuffer Getter
		/// @return RAII pointer to the vertex buffer
		const vk::raii::Buffer *GetVertexBuffer();

		/// @brief DeviceMemory Getter
		/// @return RAII pointer to the device memory
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
