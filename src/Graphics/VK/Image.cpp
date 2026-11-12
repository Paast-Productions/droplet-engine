#include <Graphics/VK/Image.hpp>

#include <print>
#include <iostream>
#include <Graphics/VK/BufferHelper.hpp>

using namespace Droplet::Graphics::VK;

Image::Image(const vma::raii::Allocator &p_allocator,
			 const vk::raii::CommandBuffer &p_commandBuffer,
			 const std::byte *p_pixels,
			 const std::size_t p_size,
			 std::uint32_t p_width, 
			 std::uint32_t p_height, 
			 vk::Format p_format, 
			 vk::ImageTiling p_tiling, 
			 vk::ImageUsageFlags p_usage, 
			 vk::MemoryPropertyFlags p_properties,
			 vk::PipelineStageFlagBits2 p_dstStage,
			 vk::AccessFlagBits2 p_dstAccess,
			 vk::ImageLayout p_newLayout) :
				m_extent(p_width, p_height),
				m_format(p_format)
{
	
	
	vk::DeviceSize imageSize = p_size;
	
	// --------------
	// Staging Buffer
	// --------------
	
	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = p_size,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};
	
	vma::AllocationCreateInfo allocationCreateInfo
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eMapped,
		.usage = vma::MemoryUsage::eAutoPreferHost,
	};
	
	vma::raii::Buffer stagingBuffer = vma::raii::Buffer { p_allocator, bufferCreateInfo, allocationCreateInfo };
	
	{
		const vma::raii::Allocation &stagingAllocation = stagingBuffer.getAllocation();
		void* mappedMemory { stagingAllocation.map() };
		std::memcpy(mappedMemory, p_pixels, p_size);
		stagingAllocation.unmap();
	}
	
	// -----
	// Image
	// -----
	
	const vk::ImageCreateInfo imageInfo
	{ 
		.imageType = vk::ImageType::e2D,
		.format = p_format,
		.extent = 
		{
			.width = p_width, 
			.height = p_height, 
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = p_tiling,
		.usage = p_usage,
		.sharingMode = vk::SharingMode::eExclusive, 
		.initialLayout = vk::ImageLayout::eUndefined
	};	
	
	const vma::AllocationCreateInfo allocCreateInfo 
	{
		.usage = vma::MemoryUsage::eAutoPreferDevice,
		.requiredFlags = p_properties | vk::MemoryPropertyFlagBits::eDeviceLocal,
	};
	
	m_image = vma::raii::Image { p_allocator, imageInfo, allocCreateInfo };
	
	// ---------------
	// Memory Barriers
	// ---------------
	
	vk::ImageMemoryBarrier2 preCopyBarrier
	{
		.srcStageMask = {},
		.srcAccessMask = {},
		.dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
		.dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
		.oldLayout = {},
		.newLayout = vk::ImageLayout::eTransferDstOptimal,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = m_image,
		.subresourceRange =
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	
	vk::ImageMemoryBarrier2 postCopyBarrier 
	{
		.srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
		.srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
		.dstStageMask = p_dstStage,
		.dstAccessMask = p_dstAccess,
		.oldLayout = vk::ImageLayout::eTransferDstOptimal,
		.newLayout = p_newLayout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = m_image,
		.subresourceRange =
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	
	std::vector<vk::ImageMemoryBarrier2> barriers { preCopyBarrier, postCopyBarrier };
	
	vk::DependencyInfo imageDependencyInfo
	{
		.imageMemoryBarrierCount = 2,
		.pImageMemoryBarriers = barriers.data(),
	};
	
	p_commandBuffer.pipelineBarrier2(imageDependencyInfo);
	
	// ------------
	// Copy Command
	// ------------
	
	vk::BufferImageCopy2 bufferImageCopy 
	{
		.bufferOffset = 0,
		.imageSubresource = 
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1
		},
		.imageOffset = 
		{
			.x = 0,
			.y = 0,
			.z = 0
		},
		.imageExtent = 
		{
			.width = p_width,
			.height = p_height,
			.depth = 1
		}
	};
	
	vk::CopyBufferToImageInfo2 copyBufferToImageInfo
	{
		.sType = vk::StructureType::eCopyBufferToImageInfo2,
		.srcBuffer = stagingBuffer,
		.dstImage = m_image,
		.dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
		.regionCount = 1,
		.pRegions = &bufferImageCopy
	};
	
	p_commandBuffer.copyBufferToImage2(copyBufferToImageInfo);
}

Image::Image(const vma::raii::Allocator &p_allocator,
					 std::uint32_t p_width, 
					 std::uint32_t p_height,
					 vk::Format p_format,
					 vk::ImageTiling p_tiling, 
					 vk::ImageUsageFlags p_usage, 
					 vk::MemoryPropertyFlags p_properties) :
						m_extent(p_width, p_height),
						m_format(p_format)
{
	vk::ImageCreateInfo imageInfo
	{
		.imageType = vk::ImageType::e2D,
		.format = p_format,
		.extent = 
		{
			.width = p_width,
			.height = p_height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = p_tiling,
		.usage = p_usage,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined
	};
	
	const vma::AllocationCreateInfo allocCreateInfo 
	{
		//.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = p_properties
	};
	try
	{
		m_image = vma::raii::Image {p_allocator, imageInfo, allocCreateInfo};
	} catch (const std::exception &e)
	{
		std::print(std::cerr, "Error: {0}", e.what());
		std::flush(std::cerr);
	}
}

void Image::TransitionLayout(
	const vk::raii::CommandBuffer &p_commandBuffer,
	vk::ImageLayout p_newLayout, 
	vk::PipelineStageFlagBits2 p_srcStage,
	vk::PipelineStageFlagBits2 p_dstStage,
	vk::AccessFlagBits2 p_srcAccess,
	vk::AccessFlagBits2 p_dstAccess)
{
	vk::ImageMemoryBarrier2 barrier
	{ 
		.srcStageMask = p_srcStage,
		.srcAccessMask = p_srcAccess,
		.dstStageMask = p_dstStage,
		.dstAccessMask = p_dstAccess,
		.oldLayout = m_currentLayout,
		.newLayout = p_newLayout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = m_image,
		.subresourceRange = 
		{
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		} 
	};

	vk::DependencyInfo dependencyInfo
	{
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};
	
	p_commandBuffer.pipelineBarrier2(dependencyInfo);
	
	m_currentLayout = p_newLayout;
}
