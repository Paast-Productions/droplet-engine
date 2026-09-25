#include "Allocator.hpp"

using namespace Droplet::Graphics::VK;

Allocator::Allocator(const vk::raii::Instance &p_instance, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device)
{
        // HACK: I don't know how to make this C++ vk:: / vk::raii::
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
