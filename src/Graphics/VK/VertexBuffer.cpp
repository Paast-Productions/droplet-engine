#include "VertexBuffer.hpp"

using namespace Droplet::Graphics::VK;

VertexBuffer::VertexBuffer(const vma::raii::Allocator &p_allocator, const std::vector<Vertex> &p_vertices)
{
	const vk::DeviceSize bufferSize { sizeof(p_vertices[0]) * p_vertices.size() };
	
	const vk::BufferCreateInfo bufferCreateInfo
	{
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc,
		.sharingMode = vk::SharingMode::eExclusive
	};
	
	constexpr vma::AllocationCreateInfo allocCreateInfo
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};
	
	m_vertexBuffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocCreateInfo};
	
	const vma::raii::Allocation &allocation = m_vertexBuffer.getAllocation();
	allocation.copyFromMemory(p_vertices.data(), 0, bufferSize);
}