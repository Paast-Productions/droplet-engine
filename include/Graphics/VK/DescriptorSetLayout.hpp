#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout() = delete;
        
        /// @brief Constructs a vk::DescriptorSetLayout with a bindless setup
        /// @param p_device Vulkan Device
        DescriptorSetLayout(const vk::raii::Device& p_device);
        
        DescriptorSetLayout(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout &operator=(const DescriptorSetLayout &) = delete;
        DescriptorSetLayout(DescriptorSetLayout &&) = delete;
        DescriptorSetLayout &operator=(DescriptorSetLayout &&) = delete;

        ~DescriptorSetLayout() = default;
        
        [[nodiscard]] const vk::raii::DescriptorSetLayout &Get() const;
    
    private:
        vk::raii::DescriptorSetLayout m_descriptorSetLayout {nullptr};
    };

    inline const vk::raii::DescriptorSetLayout &DescriptorSetLayout::Get() const
    {
        return m_descriptorSetLayout;
    }
}
