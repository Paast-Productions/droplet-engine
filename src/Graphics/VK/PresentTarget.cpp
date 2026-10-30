#include "PresentTarget.hpp"

using namespace Droplet::Graphics::VK;

PresentTarget::PresentTarget(
    const vma::raii::Allocator &p_allocator, 
    const vk::raii::Device &p_device, 
    const vk::raii::PhysicalDevice &p_physicalDevice, 
    const vk::Image p_presentImage, 
    const vk::Format p_surfaceFormat, 
    const std::uint32_t p_width,
    const std::uint32_t p_height) :
        m_extent(p_width, p_height),
        m_imageView(p_device, p_presentImage, p_surfaceFormat, vk::ImageAspectFlagBits::eColor),
        m_depthBuffer(p_allocator, p_device, p_physicalDevice, m_extent) {}

