#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <vector>
#include <glm/glm.hpp>

#include <Graphics/VK/CommandPool.hpp>

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
		VertexBuffer() = delete;
		
		/// @brief
		VertexBuffer(nullptr_t p_nullptr)
		{
			m_vertexBuffer = { p_nullptr };
		}

		/// @brief Constructor for the VertexBuffer class
		/// @param p_allocator VMA RAII Allocator 
		/// @param p_vertices vector containing Vertex struct data
		VertexBuffer(const vma::raii::Allocator &p_allocator,
			const std::vector<Vertex> &p_vertices);
		
		VertexBuffer(const VertexBuffer &) = delete;
		VertexBuffer &operator=(const VertexBuffer &) = delete;
		
		VertexBuffer(VertexBuffer &&p_other) noexcept
		{
			m_vertexBuffer = std::move(p_other.m_vertexBuffer);
		}
		
		VertexBuffer &operator=(VertexBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			m_vertexBuffer == std::move(p_other.m_vertexBuffer);
			
			return *this;
		}

		~VertexBuffer() = default;
		
		bool operator==(const VertexBuffer &p_other) const
		{
			return m_vertexBuffer == p_other.m_vertexBuffer;
		}
		
		/// @brief VertexBuffer Getter
		/// @return RAII pointer to the vertex buffer
		[[nodiscard]] const vma::raii::Buffer &Get();
		
	private:
		vma::raii::Buffer m_vertexBuffer { nullptr };
	};

	/// @brief Getter for Vertex buffer reference
	/// @return Vertex buffer pointer
	inline const vma::raii::Buffer &VertexBuffer::Get()
	{
		return m_vertexBuffer;
	}
}
