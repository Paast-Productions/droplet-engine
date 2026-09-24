#include <vulkan/vulkan_raii.hpp>

#include <Graphics/VK/DepthBuffer.hpp>

using namespace Droplet::Graphics::VK;

DepthBuffer::DepthBuffer(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::Extent2D &p_swapchainExtent)
{
	vk::Format depthFormat = FindSupportedFormat(
		p_physicalDevice,
		{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);

	m_imageView.emplace(p_device, p_physicalDevice, p_swapchainExtent.width, 
		p_swapchainExtent.height, depthFormat, vk::ImageAspectFlagBits::eDepth, 
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, 
		vk::MemoryPropertyFlagBits::eDeviceLocal);
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