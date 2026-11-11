#include "VertexBuffer.hpp"

using namespace Droplet::Graphics::VK;

VertexBuffer::VertexBuffer(const vma::raii::Allocator &p_allocator, const std::vector<std::byte> &p_vertices)
{
	const vk::DeviceSize bufferSize { sizeof(std::byte) * p_vertices.size() };
	
	// --------------
	// Staging Buffer
	// --------------
	
	const vk::BufferCreateInfo stagingBufferCreateInfo 
	{
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};
	
	constexpr vma::AllocationCreateInfo stagingBufferAllocationInfo 
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
		.usage = vma::MemoryUsage::eAutoPreferHost,
	};
		
	vma::raii::Buffer stagingBuffer { p_allocator, stagingBufferCreateInfo, stagingBufferAllocationInfo };
	
	const vma::Allocation stagingAllocation = stagingBuffer.getAllocation();
	stagingAllocation.map();
	//std::memcpy(stagingBuffer.getAllocation().getInfo().pMappedData, p_vertices.data(), bufferSize);
	
	// -------------
	// Vertex Buffer
	// -------------
	
	const vk::BufferCreateInfo vertexBufferCreateInfo
	{
		.size = bufferSize,
		.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		.sharingMode = vk::SharingMode::eExclusive
	};
	
	constexpr vma::AllocationCreateInfo vertexBufferAllocationInfo
	{
		.flags = vma::AllocationCreateFlagBits::eDedicatedMemory,
		.usage = vma::MemoryUsage::eAutoPreferDevice,
		.requiredFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
	};
	
	m_vertexBuffer = vma::raii::Buffer {p_allocator, vertexBufferCreateInfo, vertexBufferAllocationInfo};
	
	// Barriers
	
	vk::BufferMemoryBarrier2 copyBarrier
	{
		.srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
		.srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
		.dstStageMask = vk::PipelineStageFlagBits2::eVertexAttributeInput,
		.dstAccessMask = vk::AccessFlagBits2::eVertexAttributeRead,
		.buffer = m_vertexBuffer, 
		.offset = 0,
		.size = vk::WholeSize,
	};
	
	vk::DependencyInfo dependencyInfo 
	{
		.bufferMemoryBarrierCount = 1,
		.pBufferMemoryBarriers = &copyBarrier,
	};
}