#include <RenderTarget.hpp>

using namespace Droplet::Graphics;

RenderTarget::RenderTarget(
	const vma::raii::Allocator &p_allocator,
	const vk::raii::Device &p_device,
	const vk::raii::PhysicalDevice &p_physicalDevice,
	uint32_t p_width,
	uint32_t p_height)
	: m_extent{p_width, p_height},
	m_colorImage(
		p_allocator,
		p_width,
		p_height,
		vk::Format::eB8G8R8A8Unorm,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal),
	m_colorView(
		p_device,
		m_colorImage.Get()
	),
	m_depthBuffer(
		p_allocator,
		p_device,
		p_physicalDevice,
		m_extent
	)

{

}


VK::Image &RenderTarget::GetColorImage()
{
	return m_colorImage;
}
VK::ImageView &RenderTarget::GetColorImageView()
{
	return m_colorView;
}

VK::DepthBuffer &RenderTarget::GetDepthBuffer()
{
	return m_depthBuffer;
}
vk::Extent2D RenderTarget::GetExtent() const
{
	return m_extent;
}