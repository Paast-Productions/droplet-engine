#include <GraphicsPCH.h>
#include <Graphics/Vulkan/Resource/Buffer.hpp>

using namespace Droplet::Graphics::Vulkan;

Buffer::Buffer(const vma::raii::Allocator &p_allocator,
	const vk::raii::Device &p_device,
	const vk::BufferCreateInfo &p_createInfo,
	const vma::AllocationCreateInfo &p_allocationInfo,
	[[maybe_unused]] const std::string &p_bufferName) :
		m_size(p_createInfo.size),
		m_usage(p_createInfo.usage)
{
	if (p_createInfo.size == 0 || !p_createInfo.usage)
	{
		throw std::invalid_argument("Invalid buffer create info");
	}

	m_buffer = p_allocator.createBuffer(p_createInfo, p_allocationInfo);
	
#ifdef _DEBUG
	p_device.setDebugUtilsObjectNameEXT(*m_buffer, p_bufferName);
#endif
}

Buffer::Buffer(const vma::raii::Allocator &p_allocator,
	const vk::raii::Device& p_device,
	const BufferCreateOptions &p_options,
	[[maybe_unused]] const std::string &p_bufferName)
{
	if (p_options.size == 0 || !p_options.usage)
	{
		throw std::invalid_argument("Invalid buffer create options");
	}

	vk::BufferCreateInfo info{};
	info.flags = p_options.flags;
	info.size = p_options.size;
	info.usage = p_options.usage;
	info.sharingMode = vk::SharingMode::eExclusive;

	*this = Buffer { p_allocator, p_device, info, p_options.allocationInfo, p_bufferName };
}

void Buffer::HostWrite(const void *p_data,
	const vk::DeviceSize p_size,
	const vk::DeviceSize p_offset) const
{
	if (p_size == 0)
		return;

	if (!p_data)
	{
		throw std::invalid_argument("Cannot write data from a nullptr");
	}

	if (p_offset > m_size || p_size > m_size - p_offset)
	{
		throw std::out_of_range("Buffer write exceeds buffer size");
	}

	if (!IsHostVisible())
	{
		throw std::runtime_error("Cannot write to buffer not visible to host");
	}
	
	{
		void* mappedMem { m_buffer.getAllocation().map() };
		std::memcpy(mappedMem, p_data, p_size + p_offset);
		m_buffer.getAllocation().unmap();
	}
}

void Buffer::Upload(const Buffer &p_stagingBuffer,
	const vk::raii::CommandBuffer &p_commandBuffer,
	const vk::DependencyInfo &p_dependencyInfo,
	const void *p_data,
	const vk::DeviceSize p_size,
	const vk::DeviceSize p_offset) const
{
	if (!(m_usage & vk::BufferUsageFlagBits::eTransferDst))
	{
		throw std::logic_error("Buffer is not host-visible and lacks eTransferDst");
	}
	
	if (p_offset > m_size || p_size > m_size - p_offset)
	{
		throw std::out_of_range("Buffer upload exceeds buffer size");
	}

	p_stagingBuffer.HostWrite(p_data, p_size);
	
	vk::BufferCopy2 copyRegion
	{
		.srcOffset = 0,
		.dstOffset = p_offset,
		.size = p_size
	};
	
	vk::CopyBufferInfo2 copyBufferInfo 
	{
		.srcBuffer = p_stagingBuffer.Get(),
		.dstBuffer = Get(),
		.regionCount = 1,
		.pRegions = &copyRegion
	};
	
	p_commandBuffer.copyBuffer2(copyBufferInfo);
	p_commandBuffer.pipelineBarrier2(p_dependencyInfo);
}
