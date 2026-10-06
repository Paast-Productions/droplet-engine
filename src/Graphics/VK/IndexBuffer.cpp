#include "IndexBuffer.hpp"
#include <iostream>


using namespace Droplet::Graphics::VK;

IndexBuffer::IndexBuffer(const vma::raii::Allocator &p_allocator, const std::vector<uint16_t> &p_indices)
{
	//std::cout << G_INDICES << std::endl;
	vk::DeviceSize bufferSize = sizeof(p_indices[0]) * p_indices.size();
	
	const vk::BufferCreateInfo bufferCreateInfo 
	{
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eIndexBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};
	
	constexpr vma::AllocationCreateInfo allocCreateInfo 
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};
	
	m_indexBuffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocCreateInfo};
	
	const vma::raii::Allocation& allocation = m_indexBuffer.getAllocation();
	allocation.copyFromMemory(p_indices.data(), 0, bufferSize);
}