#include <RenderTarget.hpp>

using namespace Droplet::Graphics;

RenderTarget::RenderTarget(
	const vma::raii::Allocator &p_allocator,
	const vk::raii::Device &p_device,
	const vk::raii::PhysicalDevice &p_physicalDevice,
	const std::uint32_t p_width,
	const std::uint32_t p_height) : 
		m_extent(p_width, p_height),
		m_colourImage(p_allocator, p_width, p_height, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal),
		m_colourView(p_device, m_colourImage.Get()),
		m_depthBuffer(p_allocator, p_device, p_physicalDevice, m_extent) {}