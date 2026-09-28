#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vk_mem_alloc_raii.hpp>
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
		/// @param p_allocator Global VMA Allocator
		UniformBuffer(const vma::raii::Allocator &p_allocator);
		
		~UniformBuffer() = default;

		/// @brief Updates the buffer
		/// @param p_swapchainExtent The extent of the swapchain
		void UpdateBuffer(const vk::Extent2D &p_swapchainExtent) const;

		/// @brief Buffer Getter
		/// @return RAII pointer to the VMA uniform buffer
		const vma::raii::Buffer &GetBuffer();
		
	private:

		vma::raii::Buffer m_buffer { nullptr };
	};

	inline const vma::raii::Buffer &UniformBuffer::GetBuffer()
	{
		return m_buffer;
	}
}

