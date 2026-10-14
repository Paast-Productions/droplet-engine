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
		/// @brief Deleted constructor
		IndexBuffer() = delete;
		
		IndexBuffer(nullptr_t p_nullptr)
		{
			m_indexBuffer = { p_nullptr };
		}
		
		/// @brief Index Buffer constructor
		/// @param p_allocator VMA RAII Allocator
		/// @param p_indices Vector containing index data
		IndexBuffer(const vma::raii::Allocator &p_allocator,
			const std::vector<std::uint16_t> &p_indices);
		
		IndexBuffer(const IndexBuffer &) = delete;
		IndexBuffer &operator=(const IndexBuffer &) = delete;
		
		IndexBuffer(IndexBuffer &&p_other) noexcept
		{
			m_indexBuffer = std::move(p_other.m_indexBuffer);
		}
		
		IndexBuffer &operator=(IndexBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			m_indexBuffer = std::move(p_other.m_indexBuffer);
			
			return *this;
		}
		
		/// @brief Default destructor
		~IndexBuffer() = default;
		
		bool operator==(const IndexBuffer &p_other) const
		{
			return m_indexBuffer == p_other.m_indexBuffer;
		}
		
		/// @brief Getter-function for the index buffer
		/// @returns Vulkan Buffer Pointer
		[[nodiscard]] const vk::raii::Buffer &Get() const { return m_indexBuffer; }
		
	private:
		vma::raii::Buffer m_indexBuffer { nullptr };
		
	};
}