#include "CommandPool.hpp"

#include <utility>

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

Droplet::Graphics::VK::CommandBuffer Droplet::Graphics::VK::CommandPool::BeginSingleTime()
{
	CommandBuffer commandBuffer {
		m_device,
		m_commandPool,
		vk::CommandBufferLevel::ePrimary
	};
	commandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	return commandBuffer;
}

void Droplet::Graphics::VK::CommandPool::EndSingleTime(CommandBuffer& p_commandBuffer,
	const vk::raii::Queue &p_queue)
{
	p_commandBuffer.End();
	
	vk::SubmitInfo submitInfo {
		.commandBufferCount = 1,
		.pCommandBuffers = &*p_commandBuffer.Get()
	};
	
	p_queue.submit(submitInfo);
	p_queue.waitIdle();
}

template<typename RecordFunction, typename... Args>
void Droplet::Graphics::VK::CommandPool::ImmediateSubmit(vk::raii::Queue &p_queue, RecordFunction&& p_recordFunction, Args&& p_args)
{
	CommandBuffer commandBuffer{m_device, m_commandPool, vk::CommandBufferLevel::ePrimary};
	commandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	
	std::invoke(
		std::forward<RecordFunction>(p_recordFunction),
		commandBuffer,
		std::forward<Args>(p_args)...
	);
	
	commandBuffer.End();
	
	vk::SubmitInfo submitInfo {
		.commandBufferCount = 1,
		.pCommandBuffers = &*commandBuffer.Get()
	};
	
	p_queue.submit(submitInfo);
	p_queue.waitIdle();
}
