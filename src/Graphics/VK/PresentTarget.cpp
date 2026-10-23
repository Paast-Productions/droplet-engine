#include "PresentTarget.hpp"

using namespace Droplet::Graphics::VK;

PresentTarget::PresentTarget(
    const vma::raii::Allocator &p_allocator, 
    const vk::raii::Device &p_device, 
    const vk::raii::PhysicalDevice &p_physicalDevice, 
    vk::Image p_presentImage, 
    vk::Format p_surfaceFormat, 
    vk::Extent2D p_extent) :
        m_extent(p_extent),
        m_imageView(p_device, p_presentImage, p_surfaceFormat, vk::ImageAspectFlagBits::eColor),
        m_depthBuffer(p_allocator, p_device, p_physicalDevice, m_extent) {}

