#include "VertexBuffer.hpp"
#include "BufferHelper.hpp"

using namespace Droplet::Graphics::VK;

VertexBuffer::VertexBuffer(vk::raii::Device const &p_device, vk::raii::PhysicalDevice const &p_physDevice, vk::raii::CommandPool const &p_commandPool, vk::raii::Queue const &p_queue, const std::vector<Vertex> &p_vertices)
{
	vk::DeviceSize bufferSize = sizeof(p_vertices[0]) * p_vertices.size();

	auto [buffer, bufferMemory] = 
		CreateBuffer(p_device, 
					 p_physDevice, 
					 bufferSize, 
					 vk::BufferUsageFlagBits::eTransferSrc,
					 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* dataStaging = bufferMemory.mapMemory(0, bufferSize);
	memcpy(dataStaging, p_vertices.data(), bufferSize);
	bufferMemory.unmapMemory();

	std::tie(m_vertexBuffer, m_bufferMemory) =
		CreateBuffer(p_device, 
					 p_physDevice, 
					 bufferSize, 
					 vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, 
					 vk::MemoryPropertyFlagBits::eDeviceLocal);

	CopyBuffer(p_device, p_queue, p_commandPool, buffer, m_vertexBuffer, bufferSize);
}