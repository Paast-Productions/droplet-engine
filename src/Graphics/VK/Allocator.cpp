#include "Allocator.hpp"

using namespace Droplet::Graphics::VK;

Allocator::Allocator(const nullptr_t p_nullptr)
{
    m_allocator = { vma::raii::Allocator{p_nullptr} };
}

Allocator::Allocator(const vk::raii::Instance &p_instance, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device)
{
    vma::VulkanFunctions vulkanFunctions 
    {
        .vkGetInstanceProcAddr = &vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = &vkGetDeviceProcAddr
    };
    
    vma::AllocatorCreateInfo allocatorCreateInfo 
    {
        .flags = vma::AllocatorCreateFlagBits::eExtMemoryBudget,
        .physicalDevice = p_physicalDevice,
        .device = p_device,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = p_instance,
        .vulkanApiVersion = VK_API_VERSION_1_4,
    };
    
    m_allocator = { vma::raii::Allocator {p_instance, p_device, allocatorCreateInfo} };
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
