#include "CommandPool.hpp"

Droplet::Graphics::VK::CommandPool::CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex,
	const vk::CommandPoolCreateFlagBits p_flags)
{
	vk::CommandPoolCreateInfo createInfo = {.flags = p_flags, .queueFamilyIndex = p_queueFamilyIndex};
	m_commandPool = vk::raii::CommandPool(p_device, createInfo);
}
