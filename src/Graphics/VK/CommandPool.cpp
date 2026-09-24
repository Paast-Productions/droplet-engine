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
}

template<typename RecordFunction>
requires std::invocable<RecordFunction&, Droplet::Graphics::VK::CommandBuffer&>
void Droplet::Graphics::VK::CommandPool::ImmediateSubmit(const vk::raii::Queue &p_queue, RecordFunction p_recordFunction)
{
	CommandBuffer commandBuffer{m_device, m_commandPool, vk::CommandBufferLevel::ePrimary};
	commandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	
	p_recordFunction(commandBuffer);
	
	commandBuffer.End();
	
	vk::SubmitInfo submitInfo {
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffer.Get()
	};
	
	p_queue.submit(submitInfo);
	p_queue.waitIdle(); // TODO: There is a risk that this becomes a bottleneck. Reconsider the suitability further up.
}
