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
		
		/// @brief Uniform Buffer constructor
		/// @param p_allocator Global VMA Allocator
		/// @param p_data Data to store in the buffer
		template <typename T>
		UniformBuffer(const vma::raii::Allocator &p_allocator, const T &p_data)
		{
			m_bufferSize = sizeof p_data;
			
			vk::BufferCreateInfo bufferCreateInfo
			{
				.size = m_bufferSize, 
				.usage = vk::BufferUsageFlagBits::eUniformBuffer,
				.sharingMode = vk::SharingMode::eExclusive
			};

			vma::AllocationCreateInfo allocCreateInfo
			{
				.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
				.usage = vma::MemoryUsage::eAuto,
				.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
			};
	
			m_buffer = vma::raii::Buffer{p_allocator, bufferCreateInfo, allocCreateInfo};
	
			UpdateBuffer(p_data);
		}
		
		~UniformBuffer() = default;

		/// @brief Updates the buffer
		/// @param p_data The data to 
		template <typename T>
		void UpdateBuffer(const T& p_data) const
		{
			if (sizeof p_data > m_bufferSize)
			{
				throw std::runtime_error(std::format("[Uniform Buffer: ERROR]\nData to be copied into buffer is bigger than the buffer!\nSize of buffer: {0}\nSize of data: {1}", m_bufferSize, sizeof p_data));
			}
			
			vma::AllocationInfo allocInfo = m_buffer.getAllocation().getInfo();
			
			std::memcpy(allocInfo.pMappedData, &p_data, sizeof p_data);
		}

		/// @brief Buffer Getter
		/// @returns RAII pointer to the VMA uniform buffer
		[[nodiscard]] const vma::raii::Buffer &Get();
		
		/// @brief Buffer Size Getter
		/// @returns Size of the VMA buffer
		[[nodiscard]] std::size_t Size();
		
	private:
		vma::raii::Buffer m_buffer { nullptr };
		std::size_t m_bufferSize { 0 };
		
	};

	inline const vma::raii::Buffer &UniformBuffer::Get()
	{
		return m_buffer;
	}
}

