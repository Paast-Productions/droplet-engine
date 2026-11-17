#include "Allocator.hpp"

using namespace Droplet::Graphics::Vulkan;

Allocator::Allocator(const vk::raii::Instance &p_instance,
    const vk::raii::PhysicalDevice &p_physicalDevice,
    const vk::raii::Device &p_device)
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
