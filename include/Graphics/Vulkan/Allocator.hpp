#pragma once

#include <vk_mem_alloc_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
    class Allocator
    {
    public:
        Allocator() = delete;
        
        /// @brief Allocator nullptr constructor
        /// @param p_nullptr nullptr
        Allocator(nullptr_t p_nullptr) :
            m_allocator(p_nullptr) {}
        
        /// @brief Constructor for a Vulkan Memory Allocator... Allocator
        /// @param p_instance Vulkan Instance
        /// @param p_physicalDevice Vulkan Physical Device
        /// @param p_device Vulkan Logical Device
        Allocator(const vk::raii::Instance &p_instance,
            const vk::raii::PhysicalDevice &p_physicalDevice,
            const vk::raii::Device &p_device);
    
        Allocator(const Allocator &) = delete;
        Allocator &operator=(const Allocator &) = delete;
        
        Allocator(Allocator &&p_other) noexcept
        {
            std::swap(this->m_allocator, p_other.m_allocator);
        }
        
        Allocator &operator=(Allocator &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
    
            std::swap(this->m_allocator, p_other.m_allocator);
    
            return *this;
        }
        
        ~Allocator() = default;
        
        bool operator==(const Allocator &p_other) const
        {
            return this->m_allocator == p_other.m_allocator;
        }
        
        /// @brief Getter-function for a vma allocator
        /// @returns vma allocator
        [[nodiscard]] const vma::raii::Allocator &Get() const { return m_allocator; }
    
    private:
    
        vma::raii::Allocator m_allocator { nullptr };
    };
}

