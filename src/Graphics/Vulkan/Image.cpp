#include "Image.hpp"

#include <Graphics/Vulkan/Context.hpp>
#include <Graphics/Vulkan/CommandPool.hpp>
#include <Graphics/Vulkan/BufferHelper.hpp>
#include <Graphics/Vulkan/TextureFormatUtils.hpp>

#include <algorithm>
#include <bit>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace Droplet::Graphics::Vulkan;

Image::Image(const vma::raii::Allocator &p_allocator,
	const CommandPool &p_commandPool,
	const Context &p_context,
	const std::byte *p_pixels,
	const std::size_t p_size,
	std::uint32_t p_width, 
	std::uint32_t p_height, 
	vk::Format p_format, 
	vk::ImageTiling p_tiling, 
	vk::ImageUsageFlags p_usage, 
	vk::MemoryPropertyFlags p_properties,
	std::uint32_t p_dataMipLevels,
	bool p_generateMipmaps) :
		m_extent(p_width, p_height),
		m_format(p_format)
{
	const std::uint32_t dataLevels = std::max(1u, p_dataMipLevels);
	const TextureFormatUtils::BlockLayout layout = TextureFormatUtils::GetBlockLayout(p_format);

	const vk::FormatProperties formatProperties = p_context.GetPhysicalDevice().getFormatProperties(p_format);
	const vk::FormatFeatureFlags formatFeatures = p_tiling == vk::ImageTiling::eOptimal
		? formatProperties.optimalTilingFeatures
		: formatProperties.linearTilingFeatures;
	const bool canLinearBlit = static_cast<bool>(formatFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear);
	const vk::Filter blitFilter = canLinearBlit ? vk::Filter::eLinear : vk::Filter::eNearest;

	// Blits cannot read or write block-compressed formats, so those keep only the mips they were loaded with.
	const bool canBlit = layout.blockWidth == 1 && layout.blockHeight == 1
		&& static_cast<bool>(formatFeatures & vk::FormatFeatureFlagBits::eBlitSrc)
		&& static_cast<bool>(formatFeatures & vk::FormatFeatureFlagBits::eBlitDst);
	const bool generateMipmaps = p_generateMipmaps && canBlit;
	if (p_generateMipmaps && !generateMipmaps)
		std::cerr << "Mipmap generation skipped: format does not support blitting" << std::endl;

	const std::uint32_t fullMipLevels = generateMipmaps
		? std::max(dataLevels, static_cast<std::uint32_t>(std::bit_width(std::max(p_width, p_height))))
		: dataLevels;
	m_mipLevels = fullMipLevels;

	if (fullMipLevels > 1)
		p_usage |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

	// Data mip levels are packed tightly, largest first.
	std::vector<vk::BufferImageCopy> bufferImageCopies;
	vk::DeviceSize dataOffset = 0;
	for (std::uint32_t level = 0; level < dataLevels; ++level)
	{
		const std::uint32_t levelWidth = std::max(1u, p_width >> level);
		const std::uint32_t levelHeight = std::max(1u, p_height >> level);
		const vk::DeviceSize levelBytes = static_cast<vk::DeviceSize>((levelWidth + layout.blockWidth - 1) / layout.blockWidth)
			* ((levelHeight + layout.blockHeight - 1) / layout.blockHeight) * layout.blockBytes;

		bufferImageCopies.push_back(vk::BufferImageCopy
		{
			.bufferOffset = dataOffset,
			.imageSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = level, .baseArrayLayer = 0, .layerCount = 1 },
			.imageExtent = { .width = levelWidth, .height = levelHeight, .depth = 1 }
		});
		dataOffset += levelBytes;
	}
	if (dataOffset != p_size)
		throw std::runtime_error("Texture data size does not match its mip chain");
	
	// CREATE NECESSARY BUFFERS
	
	vk::DeviceSize imageSize = p_size;
	
	vk::BufferCreateInfo bufferCreateInfo
	{
		.size = p_size,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};
	
	vma::AllocationCreateInfo allocationCreateInfo
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessSequentialWrite,
		.usage = vma::MemoryUsage::eAuto,
	};
	
	vma::raii::Buffer stagingBuffer = vma::raii::Buffer { p_allocator, bufferCreateInfo, allocationCreateInfo };
	const vma::raii::Allocation &allocation = stagingBuffer.getAllocation();
	
	allocation.copyFromMemory(p_pixels, 0, imageSize);
	
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
		.mipLevels = fullMipLevels,
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
	
	m_image = vma::raii::Image { p_allocator, imageInfo, allocCreateInfo };
	
	// RECORD UPLOAD, THEN GENERATE MIP LEVELS BEYOND THE DATA
	
	p_commandPool.ImmediateSubmit(p_context.GetDevice(), p_context.GetQueue(),
		[&](Droplet::Graphics::Vulkan::CommandBuffer &p_commandBuffer)
		{
			auto &&commandBuffer = p_commandBuffer.Get();

			auto recordBarrier = [&](std::uint32_t p_baseMip, std::uint32_t p_levelCount,
				vk::ImageLayout p_oldLayout, vk::ImageLayout p_newLayout,
				vk::PipelineStageFlags2 p_srcStage, vk::AccessFlags2 p_srcAccess,
				vk::PipelineStageFlags2 p_dstStage, vk::AccessFlags2 p_dstAccess)
			{
				const vk::ImageMemoryBarrier2 barrier
				{
					.srcStageMask = p_srcStage,
					.srcAccessMask = p_srcAccess,
					.dstStageMask = p_dstStage,
					.dstAccessMask = p_dstAccess,
					.oldLayout = p_oldLayout,
					.newLayout = p_newLayout,
					.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
					.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
					.image = m_image,
					.subresourceRange = { .aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = p_baseMip, .levelCount = p_levelCount, .baseArrayLayer = 0, .layerCount = 1 }
				};
				const vk::DependencyInfo dependencyInfo { .imageMemoryBarrierCount = 1, .pImageMemoryBarriers = &barrier };
				commandBuffer.pipelineBarrier2(dependencyInfo);
			};

			recordBarrier(0, fullMipLevels, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
				vk::PipelineStageFlagBits2::eTopOfPipe, {}, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite);

			commandBuffer.copyBufferToImage(stagingBuffer, m_image, vk::ImageLayout::eTransferDstOptimal, bufferImageCopies);

			// Each level is blitted from the one above it, then becomes readable by shaders.
			for (std::uint32_t level = 1; level < fullMipLevels; ++level)
			{
				const std::uint32_t srcLevel = level - 1;
				recordBarrier(srcLevel, 1, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eTransferSrcOptimal,
					vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite,
					vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead);

				if (level >= dataLevels)
				{
					const std::uint32_t srcWidth = std::max(1u, p_width >> srcLevel);
					const std::uint32_t srcHeight = std::max(1u, p_height >> srcLevel);
					const std::uint32_t dstWidth = std::max(1u, p_width >> level);
					const std::uint32_t dstHeight = std::max(1u, p_height >> level);

					vk::ImageBlit blit {};
					blit.srcSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = srcLevel, .baseArrayLayer = 0, .layerCount = 1 };
					blit.srcOffsets[1] = vk::Offset3D{ static_cast<std::int32_t>(srcWidth), static_cast<std::int32_t>(srcHeight), 1 };
					blit.dstSubresource = { .aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = level, .baseArrayLayer = 0, .layerCount = 1 };
					blit.dstOffsets[1] = vk::Offset3D{ static_cast<std::int32_t>(dstWidth), static_cast<std::int32_t>(dstHeight), 1 };
					commandBuffer.blitImage(m_image, vk::ImageLayout::eTransferSrcOptimal, m_image, vk::ImageLayout::eTransferDstOptimal, blit, blitFilter);
				}

				recordBarrier(srcLevel, 1, vk::ImageLayout::eTransferSrcOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
					vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead,
					vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderRead);
			}

			recordBarrier(fullMipLevels - 1, 1, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
				vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite,
				vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eShaderRead);
		});
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