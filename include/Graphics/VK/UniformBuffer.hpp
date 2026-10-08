#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
	/// @brief Uniform Buffer Class
	class UniformBuffer
	{
	public:
		UniformBuffer() = delete;
		
		UniformBuffer(nullptr_t p_nullptr)
		{
			m_buffer = { p_nullptr };
		}
		
		/// @brief Uniform Buffer constructor
		/// @param p_allocator Global VMA Allocator
		/// @param p_data Data to store in the buffer
		template <typename T>
		UniformBuffer(const vma::raii::Allocator &p_allocator, const T &p_data)
		{
			static_assert(sizeof(p_data) % sizeof(std::uint32_t) * 4 == 0, "[ERROR: Uniform Buffer]\nSize of p_data must be a multiple of 16 bytes");
			
			m_bufferMaxSize = sizeof(p_data);
			
			const vk::BufferCreateInfo bufferCreateInfo
			{
				.size = m_bufferMaxSize, 
				.usage = vk::BufferUsageFlagBits::eUniformBuffer,
				.sharingMode = vk::SharingMode::eExclusive
			};

			constexpr vma::AllocationCreateInfo allocCreateInfo
			{
				.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
				.usage = vma::MemoryUsage::eAuto,
				.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};
	
			m_buffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocCreateInfo};
	
			UpdateBuffer(p_data);
		}
		
		UniformBuffer(const UniformBuffer &) = delete;
		UniformBuffer &operator=(const UniformBuffer &) = delete;
		
		UniformBuffer(UniformBuffer &&p_other) noexcept
		{
			m_buffer = std::move(p_other.m_buffer);
			m_bufferMaxSize = std::move(p_other.m_bufferMaxSize);
		}
		
		UniformBuffer &operator=(UniformBuffer &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			m_buffer = std::move(p_other.m_buffer);
			m_bufferMaxSize = std::move(p_other.m_bufferMaxSize);
			
			return *this;
		}
		
		~UniformBuffer() = default;
		
		bool operator==(const UniformBuffer &p_other) const
		{
			if (m_buffer == p_other.m_buffer && m_bufferMaxSize == p_other.m_bufferMaxSize)
			{
				return true;
			}
			
			return false;
		}
		
		/// @brief Updates the buffer
		/// @param p_data The data to pass into the GPU
		template <typename T>
		void UpdateBuffer(const T& p_data) const
		{
			static_assert(sizeof(p_data) % sizeof(std::uint32_t) * 4 == 0, "[ERROR: Uniform Buffer]\nSize of p_data must be a multiple of 16 bytes");
			
			if (sizeof(p_data) > m_bufferMaxSize)
			{
				throw std::runtime_error(std::format("[ERROR: Uniform Buffer]\nData to be copied into buffer is bigger than the buffer!\nSize of buffer: {0}\nSize of data: {1}", m_bufferMaxSize, sizeof p_data));
			}
			
			const vma::raii::Allocation &allocation = m_buffer.getAllocation();
			allocation.copyFromMemory(&p_data, 0, sizeof(p_data));
		}

		/// @brief Buffer Getter
		/// @returns RAII pointer to the VMA uniform buffer
		[[nodiscard]] const vma::raii::Buffer &Get() const { return m_buffer; }
		
		/// @brief Max Buffer Size Getter
		/// @returns Max size of the VMA buffer
		[[nodiscard]] std::size_t MaxSize() const { return m_bufferMaxSize; }
		
	private:
		vma::raii::Buffer m_buffer { nullptr };
		std::size_t m_bufferMaxSize { 0 };
		
	};
}

