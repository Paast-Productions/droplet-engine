#include "CommandBuffer.hpp"

vk::raii::CommandBuffer& Droplet::Graphics::VK::CommandBuffer::Get()
{
	return m_commandBuffer;
}

void Droplet::Graphics::VK::CommandBuffer::Begin(vk::CommandBufferUsageFlagBits p_flags)
{
	vk::CommandBufferBeginInfo beginInfo { .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	m_commandBuffer.begin(beginInfo);
}

void Droplet::Graphics::VK::CommandBuffer::End()
{
	m_commandBuffer.end();
}

void Droplet::Graphics::VK::CommandBuffer::CopyBuffer(vk::Buffer p_src, vk::Buffer p_dst,
	vk::DeviceSize p_size)
{
	m_commandBuffer.copyBuffer(p_src, p_dst, vk::BufferCopy{.size = p_size});
}

Droplet::Graphics::VK::CommandBuffer::CommandBuffer(const vk::raii::Device &p_device,
                                                    const vk::raii::CommandPool &p_pool, const vk::CommandBufferLevel p_level)
{
	assert(p_pool != nullptr);
	
	vk::CommandBufferAllocateInfo allocInfo { .commandPool = p_pool,  .level =  p_level, .commandBufferCount = 1};
	m_commandBuffer = std::move(vk::raii::CommandBuffers(p_device, allocInfo).front());
}
