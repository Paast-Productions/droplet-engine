#include "StorageBuffer.hpp"

Droplet::Graphics::VK::StorageBuffer::StorageBuffer(const vma::raii::Allocator &p_allocator, const void *p_data,
	const size_t p_size)
{
	if (!p_data)
	{
		throw std::invalid_argument("[ERROR: Storage Buffer]\nData pointer must not be nullptr.");
	}
	if (p_size == 0)
	{
		throw std::invalid_argument("[ERROR: Storage Buffer]\nCannot create a storage buffer with zero size.");
	}
            
	m_bufferCapacity = p_size;
                
	const vk::BufferCreateInfo bufferCreateInfo
	{
		.size = m_bufferCapacity, 
		.usage = vk::BufferUsageFlagBits::eStorageBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};

	constexpr vma::AllocationCreateInfo allocCreateInfo
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};
        
	m_buffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocCreateInfo};
        
	UpdateBytes(p_data, p_size);
}

void Droplet::Graphics::VK::StorageBuffer::UpdateBytes(const void *p_data, std::size_t p_size)
{
	if (!p_data)
		return;
	
	if (p_size == 0)
		return;
            
	if (p_size > m_bufferCapacity)
	{
		throw std::runtime_error(std::format("[ERROR: Storage Buffer]\nData to be copied into buffer is bigger than the buffer!\nSize of buffer: {0}\nSize of data: {1}", m_bufferCapacity, p_size));
	}
                
	m_buffer.getAllocation().copyFromMemory(p_data, 0, p_size);
}
