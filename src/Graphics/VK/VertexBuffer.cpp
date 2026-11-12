#include "VertexBuffer.hpp"

using namespace Droplet::Graphics::VK;

VertexBuffer::VertexBuffer(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::CommandBuffer &p_commandBuffer, const std::vector<std::byte> &p_vertices)
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
		
	m_stagingBuffer = vma::raii::Buffer { p_allocator, stagingBufferCreateInfo, stagingBufferAllocationInfo };
	
#ifdef DEBUG
	p_device.setDebugUtilsObjectNameEXT(*m_stagingBuffer, "Staging Buffer (Vertex Buffer)");
#endif
	
	{
		const vma::raii::Allocation &stagingAllocation = m_stagingBuffer.getAllocation();
		void* mappedMem = stagingAllocation.map();
		std::memcpy(mappedMem, p_vertices.data(), bufferSize);
		stagingAllocation.unmap();
	}
	
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
	
	m_vertexBuffer = vma::raii::Buffer { p_allocator, vertexBufferCreateInfo, vertexBufferAllocationInfo };
	
#ifdef DEBUG
	p_device.setDebugUtilsObjectNameEXT(*m_vertexBuffer, "Vertex Buffer");
#endif
	
	// -----------
	// Device Copy
	// -----------
	
	vk::BufferCopy2 copyRegion
	{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = bufferSize
	};
	
	vk::CopyBufferInfo2 copyBufferInfo 
	{
		.srcBuffer = m_stagingBuffer,
		.dstBuffer = m_vertexBuffer,
		.regionCount = 1,
		.pRegions = &copyRegion
	};
	
	p_commandBuffer.copyBuffer2(copyBufferInfo);
	
	// ---------------
	// Memory Barriers
	// ---------------
	
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
	
	p_commandBuffer.pipelineBarrier2(dependencyInfo);
}