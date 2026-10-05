#include "VertexBuffer.hpp"
#include "BufferHelper.hpp"

using namespace Droplet::Graphics::VK;

VertexBuffer::VertexBuffer(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physDevice, const CommandPool &p_commandPool, const vk::raii::Queue &p_queue, const std::vector<Vertex> &p_vertices)
{
	const vk::DeviceSize bufferSize { sizeof(p_vertices[0]) * p_vertices.size() };

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

	void *dataStaging = bufferMemory.mapMemory(0, bufferSize);
	std::memcpy(dataStaging, p_vertices.data(), bufferSize);
	bufferMemory.unmapMemory();

	std::tie(m_vertexBuffer, m_bufferMemory) = CreateBuffer
	(
		p_device, 
		p_physDevice, 
		bufferSize, 
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	CopyBuffer(p_queue, p_commandPool, buffer, m_vertexBuffer, bufferSize);
}