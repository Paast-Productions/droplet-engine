#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    class Allocator
    {
    public:
        Allocator() = delete;
    
        Allocator(const vk::raii::Instance &p_instance, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device);
    
        Allocator(const Allocator &) = delete;
        Allocator &operator=(const Allocator &) = delete;
        Allocator(Allocator &&) = delete;
        Allocator &operator=(Allocator &&) = delete;

        ~Allocator() = default;
    
    private:
    
        vma::raii::Allocator m_allocator { nullptr };
    };
}

