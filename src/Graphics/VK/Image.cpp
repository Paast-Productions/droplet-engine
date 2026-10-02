#include <Graphics/VK/Image.hpp>

#include <print>
#include <iostream>
#include <Graphics/VK/BufferHelper.hpp>

using namespace Droplet::Graphics::VK;

Image::Image(const vma::raii::Allocator &p_allocator,
			 const vk::raii::CommandBuffer &p_commandBuffer,
			 const unsigned char *p_pixels, 
			 std::uint32_t p_width, 
			 std::uint32_t p_height, 
			 vk::Format p_format, 
			 vk::ImageTiling p_tiling, 
			 vk::ImageUsageFlags p_usage, 
			 vk::MemoryPropertyFlags p_properties)
{
	
	// CREATE NECESSARY BUFFERS
	
	vk::DeviceSize imageSize = static_cast<std::uint64_t>(p_width) * static_cast<std::uint64_t>(p_height) * 4;
	
	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = imageSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};
	
	vma::AllocationCreateInfo allocationCreateInfo
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
		.usage = vma::MemoryUsage::eAuto,
	};
	
	vma::raii::Buffer stagingBuffer = vma::raii::Buffer {p_allocator, bufferCreateInfo, allocationCreateInfo};
	const vma::raii::Allocation &allocation = stagingBuffer.getAllocation();
	
	allocation.copyFromMemory(p_pixels, 0, imageSize);
	
	const vk::ImageCreateInfo imageInfo
	{ 
		.flags = static_cast<vk::ImageCreateFlagBits>(0), 
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
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite | vma::AllocationCreateFlagBits::eHostAccessAllowTransferInstead | vma::AllocationCreateFlagBits::eMapped,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = p_properties,
		.preferredFlags = p_properties, 
	};
	
	m_image = vma::raii::Image {p_allocator, imageInfo, allocCreateInfo};
	
	// SETUP DEVICE BARRIERS
	
	vk::BufferMemoryBarrier2 stagingBufferBarrier
	{
		.sType = vk::StructureType::eBufferMemoryBarrier2,
		.srcStageMask = vk::PipelineStageFlagBits2::eHost,
		.srcAccessMask = vk::AccessFlagBits2::eHostWrite,
		.dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
		.dstAccessMask = vk::AccessFlagBits2::eTransferRead,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.buffer = stagingBuffer,
		.offset = 0,
		.size = vk::WholeSize
	};
	
	std::array<vk::BufferMemoryBarrier2, 1> bufferMemoryBarriers { stagingBufferBarrier }; 
	
	vk::DependencyInfo stagingDependencyInfo
	{
		.sType = vk::StructureType::eDependencyInfo,
		.bufferMemoryBarrierCount = 1,
		.pBufferMemoryBarriers = bufferMemoryBarriers.data(),
	};
	
	vk::ImageMemoryBarrier2 imageBarrier 
	{
		.sType = vk::StructureType::eImageMemoryBarrier2,
		.srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
		.srcAccessMask = {},
		.dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
		.dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
		.oldLayout = vk::ImageLayout::eUndefined,
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
	
	std::array<vk::ImageMemoryBarrier2, 1> imageMemoryBarriers { imageBarrier };
	
	vk::DependencyInfo imageDependencyInfo
	{
		.sType = vk::StructureType::eDependencyInfo,
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = imageMemoryBarriers.data(),
	};
	
	
	// PREPARE DEVICE TO DEVICE COPY
	
	vk::BufferImageCopy2 bufferImageCopy 
	{
		.sType = vk::StructureType::eBufferImageCopy2,
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
	
	std::array<vk::BufferImageCopy2, 1> bufferImageCopies { bufferImageCopy };
	
	vk::CopyBufferToImageInfo2 copyBufferToImageInfo
	{
		.sType = vk::StructureType::eCopyBufferToImageInfo2,
		.srcBuffer = stagingBuffer,
		.dstImage = m_image,
		.dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
		.regionCount = 1,
		.pRegions = bufferImageCopies.data()
	};
	
	vk::CommandBufferBeginInfo beginInfo 
	{
		.sType = vk::StructureType::eCommandBufferBeginInfo,
		.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
	};
	
	p_commandBuffer.begin(beginInfo);
	p_commandBuffer.pipelineBarrier2(stagingDependencyInfo);

	TransitionImageLayout(p_commandBuffer, m_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	p_commandBuffer.copyBufferToImage2(copyBufferToImageInfo);
	TransitionImageLayout(p_commandBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	p_commandBuffer.pipelineBarrier2(imageDependencyInfo);
	p_commandBuffer.end();
}

Image::Image(const vma::raii::Allocator &p_allocator,
					 std::uint32_t p_width, 
					 std::uint32_t p_height,
					 vk::Format p_format,
					 vk::ImageTiling p_tiling, 
					 vk::ImageUsageFlags p_usage, 
					 vk::MemoryPropertyFlags p_properties)
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