#include <Graphics/VK/Image.hpp>

#include <print>
#include <iostream>

using namespace Droplet::Graphics::VK;

Image::Image(const vma::raii::Allocator &p_allocator,
					 const unsigned char *p_pixels, 
					 std::uint32_t p_width, 
					 std::uint32_t p_height, 
					 vk::Format p_format, 
					 vk::ImageTiling p_tiling, 
					 vk::ImageUsageFlags p_usage, 
					 vk::MemoryPropertyFlags p_properties)
{
	vk::DeviceSize imageSize = static_cast<std::uint64_t>(p_width) * static_cast<std::uint64_t>(p_height) * 4;
	
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
		.initialLayout = vk::ImageLayout::eUndefined,
	};
	
	const vma::AllocationCreateInfo allocCreateInfo 
	{
		.flags = vma::AllocationCreateFlagBits::eHostAccessRandom,
		.usage = vma::MemoryUsage::eAuto,
		.requiredFlags = p_properties,
		.preferredFlags = p_properties, 
		.pool = nullptr
	};
	
	try
	{
		m_image = vma::raii::Image {p_allocator, imageInfo, allocCreateInfo};
	}
	catch (const std::exception &e)
	{
		std::print(std::cerr, "Error: {0}", e.what());
		std::flush(std::cerr);
	}
	
	const vma::raii::Allocation& allocation = m_image.getAllocation();
	allocation.copyFromMemory(p_pixels, 0, imageSize);
}

//std::pair<vk::raii::Image, vk::raii::DeviceMemory> 
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
		.sharingMode = vk::SharingMode::eExclusive 
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