#include "CommandBuffer.hpp"

vk::raii::CommandBuffer& Droplet::Graphics::VK::CommandBuffer::Get()
{
	return m_commandBuffer;
}

Droplet::Graphics::VK::CommandBuffer::CommandBuffer(const vk::raii::Device &p_device,
                                                    const vk::raii::CommandPool &p_pool, const vk::CommandBufferLevel p_level)
{
	assert(p_pool != nullptr);
	
	vk::CommandBufferAllocateInfo allocInfo { .commandPool = p_pool,  .level =  p_level, .commandBufferCount = 1};
	m_commandBuffer = std::move(vk::raii::CommandBuffers(p_device, allocInfo).front());
}
