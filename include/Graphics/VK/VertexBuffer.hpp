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
	/// The buffer currently assumes the resourcemanager will give it the same layout
	/// TODO: Make VertexBuffer vertex-layout agnostic
	struct Vertex
	{
		glm::vec3 pos;
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
		static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
		{
			return { {{.location = 0, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, pos)},
					 {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, normal)},
					 {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}} };
		}
	};

	/// @brief Vertex Buffer Class
	class VertexBuffer
	{
	public:
		VertexBuffer()
		{
			m_vertexBuffer = nullptr;
		}
		
		/// @brief
		VertexBuffer(nullptr_t p_nullptr)
		{
			m_vertexBuffer = { p_nullptr };
		}

		/// @brief Constructor for the VertexBuffer class
		/// @param p_allocator VMA RAII Allocator 
		/// @param p_commandBuffer Vulkan Command Buffer
		/// @param p_vertices vector containing Vertex struct data
		VertexBuffer(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::CommandBuffer &p_commandBuffer, const std::vector<std::byte> &p_vertices);
		
		VertexBuffer(const VertexBuffer &) = delete;
		VertexBuffer &operator=(const VertexBuffer &) = delete;
		
		VertexBuffer(VertexBuffer &&p_other) noexcept
		{
			std::swap(m_vertexBuffer, p_other.m_vertexBuffer);
			std::swap(m_stagingBuffer, p_other.m_stagingBuffer);
		}
		
		VertexBuffer &operator=(VertexBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_vertexBuffer, p_other.m_vertexBuffer);
			std::swap(m_stagingBuffer, p_other.m_stagingBuffer);
			
			return *this;
		}

		~VertexBuffer() = default;
		
		bool operator==(const VertexBuffer &p_other) const
		{
			return m_vertexBuffer == p_other.m_vertexBuffer &&
				   m_stagingBuffer == p_other.m_stagingBuffer;
		}
		
		/// @brief VertexBuffer Getter
		/// @return RAII pointer to the vertex buffer
		[[nodiscard]] const vma::raii::Buffer &Get() { return m_vertexBuffer; }
		
	private:
		vma::raii::Buffer m_vertexBuffer { nullptr };
		vma::raii::Buffer m_stagingBuffer { nullptr };
	};
}
