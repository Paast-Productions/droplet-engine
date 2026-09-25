#pragma once

#include <vector>
#include <any>

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    class DeviceMemoryAllocator
    {
    public:
        DeviceMemoryAllocator() = delete;
        
        DeviceMemoryAllocator(std::uint32_t p_minAlignment) : m_minAlignment(p_minAlignment) {}
        
        DeviceMemoryAllocator(const DeviceMemoryAllocator &) = delete;
        DeviceMemoryAllocator &operator=(const DeviceMemoryAllocator &) = delete;
        DeviceMemoryAllocator(DeviceMemoryAllocator &&) = delete;
        DeviceMemoryAllocator &operator=(DeviceMemoryAllocator &&) = delete;

        ~DeviceMemoryAllocator() = default;
        
        template<class TData>
        std::uint32_t AddRange(TData &&p_data); 
        
        void Build(const vk::raii::Device &p_device, const vma::raii::Allocator &p_allocator, const vk::raii::DescriptorPool &p_descriptorPool);
    
    private:
        [[nodiscard]] std::uint32_t PadSizeToMinAlignment(std::uint32_t p_originalSize) const;
        
        struct Range
        {
            std::uint32_t offset {};
            std::uint32_t size {};
            std::any bytes {}; 
        };
    
        std::uint32_t m_minAlignment {};
        std::uint32_t m_lastOffset {};
        std::vector<Range> m_ranges {};
        
        vk::raii::DescriptorSetLayout m_descriptorSetLayout {nullptr};
        vk::raii::DescriptorSet m_descriptorSet {nullptr};
    };
    
    inline std::uint32_t DeviceMemoryAllocator::PadSizeToMinAlignment(std::uint32_t p_originalSize) const
    {
        return (p_originalSize + m_minAlignment - 1) & ~(m_minAlignment - 1);
    }
}
