#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <vector>

namespace Droplet::Graphics::VK
{
	/// @brief Index Buffer Class
	class IndexBuffer
	{
	public:
		IndexBuffer()
		{
			m_indexBuffer = nullptr;
		}
		
		IndexBuffer(nullptr_t p_nullptr)
		{
			m_indexBuffer = { p_nullptr };
		}
		
		/// @brief Index Buffer constructor
		/// @param p_allocator VMA RAII Allocator
		/// @param p_indices Vector containing index data
		IndexBuffer(const vma::raii::Allocator &p_allocator,
			const vk::raii::Device &p_device,
			const vk::raii::CommandBuffer &p_commandBuffer,
			const std::vector<std::uint32_t> &p_indices);
		
		IndexBuffer(const IndexBuffer &) = delete;
		IndexBuffer &operator=(const IndexBuffer &) = delete;
		
		IndexBuffer(IndexBuffer &&p_other) noexcept
		{
			std::swap(m_indexCount, p_other.m_indexCount);
			std::swap(m_indexBuffer, p_other.m_indexBuffer);
			std::swap(m_stagingBuffer, p_other.m_stagingBuffer);
		}
		
		IndexBuffer &operator=(IndexBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_indexCount, p_other.m_indexCount);
			std::swap(m_indexBuffer, p_other.m_indexBuffer);
			std::swap(m_stagingBuffer, p_other.m_stagingBuffer);
			
			return *this;
		}
		
		/// @brief Default destructor
		~IndexBuffer() = default;
		
		bool operator==(const IndexBuffer &p_other) const
		{
			return m_indexBuffer == p_other.m_indexBuffer && m_stagingBuffer == p_other.m_stagingBuffer && m_indexCount == p_other.m_indexCount;
		}
		
		/// @brief Getter-function for the index buffer
		/// @returns Vulkan Buffer Pointer
		[[nodiscard]] const vk::raii::Buffer &Get() const { return m_indexBuffer; }

		/// @brief Getter function for number of indices in index buffer
		/// @return Number of indices
		[[nodiscard]] std::uint32_t GetIndexCount() const { return m_indexCount; }
		
	private:
		vma::raii::Buffer m_indexBuffer { nullptr };
		vma::raii::Buffer m_stagingBuffer { nullptr };
		std::uint32_t m_indexCount = 0;
	};
}