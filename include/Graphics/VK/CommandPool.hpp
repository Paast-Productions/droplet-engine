#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include "CommandBuffer.hpp"

namespace Droplet::Graphics::VK
{
	struct CommandBufferId
	{
		std::size_t Index;
	};
	
	class CommandPool
	{
	public:
		CommandPool() = delete;
		CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex,
			const vk::CommandPoolCreateFlags p_flags);
		
		CommandBufferId Allocate(const vk::CommandBufferLevel p_level = vk::CommandBufferLevel::ePrimary);
		
		CommandBuffer& Get(const CommandBufferId p_id);
		
	private:
		const vk::raii::Device &m_device;
		vk::raii::CommandPool m_commandPool = nullptr;
		
		std::vector<CommandBuffer> m_commandBuffers;
	};

}
