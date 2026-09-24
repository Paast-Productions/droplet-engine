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

std::vector<Droplet::Graphics::VK::CommandBufferId> Droplet::Graphics::VK::CommandPool::Allocate(uint32_t p_count,
	vk::CommandBufferLevel p_level)
{
	/* TODO: Perhaps there should be a better way of creating multiple CommandBuffer instances. Right now,
	 * the CommandBuffer constructor creates one single instance, which leads to the following implementation.
	 */
	std::vector<CommandBufferId> ids;
	for (uint32_t i = 0; i < p_count; ++i)
	{
		m_commandBuffers.push_back(CommandBuffer{m_device, m_commandPool, p_level});
		ids.push_back(CommandBufferId{ .Index = m_commandBuffers.size() - 1 });
	}
	return ids;
}

Droplet::Graphics::VK::CommandBuffer& Droplet::Graphics::VK::CommandPool::GetBuffer(const CommandBufferId p_id)
{
	assert(p_id.Index < m_commandBuffers.size());
	return m_commandBuffers[p_id.Index];
}
