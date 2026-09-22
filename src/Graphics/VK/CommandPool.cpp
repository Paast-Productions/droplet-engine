#include "CommandPool.hpp"

Droplet::Graphics::VK::CommandPool::CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex,
			const vk::CommandPoolCreateFlags p_flags) : m_device(p_device)
{
	vk::CommandPoolCreateInfo createInfo { .flags = p_flags, .queueFamilyIndex = p_queueFamilyIndex };
	m_commandPool = vk::raii::CommandPool(m_device, createInfo);
}

Droplet::Graphics::VK::CommandBufferId Droplet::Graphics::VK::CommandPool::Allocate(
	const vk::CommandBufferLevel p_level)
{
	m_commandBuffers.push_back(CommandBuffer{m_device, m_commandPool, p_level});
	return CommandBufferId{ .Index = m_commandBuffers.size() - 1 };
}

Droplet::Graphics::VK::CommandBuffer& Droplet::Graphics::VK::CommandPool::Get(const CommandBufferId p_id)
{
	assert(p_id.Index < m_commandBuffers.size());
	return m_commandBuffers[p_id.Index];
};
