#include "Allocator.hpp"

#include <print>
#include <iostream>

using namespace Droplet::Graphics::VK;

Allocator::Allocator(const nullptr_t p_nullptr)
{
    m_allocator = { vma::raii::Allocator{p_nullptr} };
}

Allocator::Allocator(const vk::raii::Instance &p_instance, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device)
{
    vma::AllocatorCreateInfo allocatorCreateInfo 
    {
        .flags = vma::AllocatorCreateFlagBits::eExtMemoryBudget,
        .physicalDevice = p_physicalDevice,
        .device = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = nullptr,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };
    
    try
    {
        m_allocator = vma::raii::Allocator { p_instance, p_device, allocatorCreateInfo };
    }
    catch (const std::exception &e)
    {
        std::print(std::cerr, "Allocator Error: {0}", e.what());
        std::flush(std::cerr);
    }
}

Allocator::Allocator(Allocator &&p_other) noexcept
{
    std::swap(this->m_allocator, p_other.m_allocator);
}

Allocator &Allocator::operator=(Allocator &&p_other) noexcept
{
    if (*this == p_other)
    {
        return *this;
    }
    
    std::swap(this->m_allocator, p_other.m_allocator);
    
    return *this;
}

bool Allocator::operator==(const Allocator &p_other) const
{
    return this->m_allocator == p_other.m_allocator;
}
