#include "IndexBuffer.hpp"
#include "BufferHelper.hpp"

using namespace Droplet::Graphics::VK;

IndexBuffer::IndexBuffer(vk::raii::Device const& p_device, vk::raii::PhysicalDevice const& p_physDevice, vk::raii::CommandPool const& p_commandPool, vk::raii::Queue const& p_queue, const std::vector<uint16_t>& p_indices)
{
	vk::DeviceSize bufferSize = sizeof(p_indices[0]) * p_indices.size();

	auto [buffer, bufferMemory] =
		CreateBuffer(p_device,
			p_physDevice,
			bufferSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	void* data = bufferMemory.mapMemory(0, bufferSize);
	memcpy(data, p_indices.data(), (size_t)bufferSize);
	bufferMemory.unmapMemory();

	std::tie(m_indexBuffer, m_bufferMemory) =
		CreateBuffer(p_device,
			p_physDevice,
			bufferSize,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	CopyBuffer(p_device, p_queue, p_commandPool, buffer, m_indexBuffer, bufferSize);
}