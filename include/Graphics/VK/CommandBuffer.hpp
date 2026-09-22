#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::VK
{
	class CommandBuffer
	{
	public:
		vk::raii::CommandBuffer& Get();
		
	private:
		friend class CommandPool;
		
		CommandBuffer() = delete;
		
		CommandBuffer(
			const vk::raii::Device &p_device,
			const vk::raii::CommandPool &p_pool,
			const vk::CommandBufferLevel p_level);
		
		vk::raii::CommandBuffer m_commandBuffer = nullptr;
	};
}
