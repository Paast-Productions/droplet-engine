#include "CommandBuffer.hpp"

void Droplet::Graphics::Vulkan::CommandBuffer::Begin(vk::CommandBufferUsageFlagBits p_flags)
{
	vk::CommandBufferBeginInfo beginInfo { .flags = p_flags};
	m_commandBuffer.begin(beginInfo);
}

void Droplet::Graphics::Vulkan::CommandBuffer::End()
{
	m_commandBuffer.end();
}

Droplet::Graphics::Vulkan::CommandBuffer::CommandBuffer(const vk::raii::Device &p_device,
	const vk::raii::CommandPool &p_pool, 
	const vk::CommandBufferLevel p_level)
{
	assert(p_pool != nullptr);
	
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = p_pool,
		.level = p_level, 
		.commandBufferCount = 1
	};
	
	m_commandBuffer = std::move(vk::raii::CommandBuffers(p_device, allocInfo).front());
}