#include "DepthBuffer.hpp"

#include <print>
#include <iostream>

#include <vulkan/vulkan_raii.hpp>


using namespace Droplet::Graphics::VK;

DepthBuffer::DepthBuffer(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::Extent2D &p_swapchainExtent)
{
	m_format = FindSupportedFormat(
		p_physicalDevice,
		{ 
			vk::Format::eD32Sfloat, 
			vk::Format::eD32SfloatS8Uint, 
			vk::Format::eD24UnormS8Uint 
		},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	
	try
	{
		m_image = {p_allocator, p_swapchainExtent.width, p_swapchainExtent.height,
			m_format, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
			vk::MemoryPropertyFlagBits::eDeviceLocal};
	} catch (const vk::Error &err)
	{
		std::print(std::cerr, "DepthBuffer-Image Error: {0}", err.what());
		std::flush(std::cerr);
		exit(-1);
	}
	
	try
	{
		m_imageView = {p_device, m_image.Get(), m_format, vk::ImageAspectFlagBits::eDepth};
	} catch (const vk::Error &err)
	{
		std::print(std::cerr, "DepthBuffer-ImageView Error: {0}", err.what());
		std::flush(std::cerr);
		exit(-1);
	}
	
}

//Check what formats the hardware supports
vk::Format DepthBuffer::FindSupportedFormat(const vk::raii::PhysicalDevice &p_physicalDevice, const std::vector<vk::Format> &p_candidates, vk::ImageTiling p_tiling, vk::FormatFeatureFlags p_features)
{
	for (const auto format : p_candidates) 
	{
		vk::FormatProperties props = p_physicalDevice.getFormatProperties(format);

		if (((p_tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & p_features) == p_features)) ||
			((p_tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & p_features) == p_features)))
		{
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}