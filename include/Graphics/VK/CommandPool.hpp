#pragma once
#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::VK
{
	class CommandPool
	{
	public:
		CommandPool() = delete;
		CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex, const vk::CommandPoolCreateFlagBits p_flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
		
	private:
		vk::raii::CommandPool m_commandPool = nullptr;
	};
}
