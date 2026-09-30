#include "IndexBuffer.hpp"
#include "BufferHelper.hpp"

using namespace Droplet::Graphics::VK;

IndexBuffer::IndexBuffer(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physDevice, const CommandPool &p_commandPool, const vk::raii::Queue &p_queue, const std::vector<uint16_t> &p_indices)
{
	vk::DeviceSize bufferSize = sizeof(p_indices[0]) * p_indices.size();
	
	// Not RAII, however no way of doing it RAII without the auto keyword
	vk::raii::Buffer buffer {nullptr}; 
	vk::raii::DeviceMemory bufferMemory {nullptr};
	
	std::tie(buffer, bufferMemory) = CreateBuffer
	(
		p_device,
		p_physDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
	
	void *data = bufferMemory.mapMemory(0, bufferSize);
	std::memcpy(data, p_indices.data(), bufferSize);
	bufferMemory.unmapMemory();

	std::tie(m_indexBuffer, m_bufferMemory) = CreateBuffer
	(
		p_device,
		p_physDevice,
		bufferSize,
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	CopyBuffer(p_queue, p_commandPool, buffer, m_indexBuffer, bufferSize);
}