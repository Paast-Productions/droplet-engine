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
        
        /// @brief Allocator nullptr constructor
        /// @param p_nullptr nullptr
        Allocator(nullptr_t p_nullptr);
    
        Allocator(const vk::raii::Instance &p_instance, const vk::raii::PhysicalDevice &p_physicalDevice, const vk::raii::Device &p_device);
    
        Allocator(const Allocator &) = delete;
        Allocator &operator=(const Allocator &) = delete;
        Allocator(Allocator &&p_other) noexcept;
        Allocator &operator=(Allocator &&p_other) noexcept;
        
        ~Allocator() = default;
        
        bool operator==(const Allocator &p_other) const;
        
        const vma::raii::Allocator &Get();
    
    private:
    
        vma::raii::Allocator m_allocator { nullptr };
    };

    inline const vma::raii::Allocator &Allocator::Get()
    {
        return m_allocator;
    }
}

