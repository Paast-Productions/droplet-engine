#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/DescriptorSetLayout.hpp>

namespace Droplet::Graphics::VK
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = delete;
        
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool& p_descriptorPool, const vk::raii::DescriptorSetLayout& p_descriptorSetLayout);
        
        DescriptorSet(const DescriptorSet &) = delete;
        DescriptorSet &operator=(const DescriptorSet &) = delete;
        DescriptorSet(DescriptorSet &&) = delete;
        DescriptorSet &operator=(DescriptorSet &&) = delete;

        ~DescriptorSet() = default;
        
        [[nodiscard]] const std::vector<vk::raii::DescriptorSet> &Get() const;
        
    private:
        std::vector<vk::raii::DescriptorSet> m_descriptorSets {};
        
    };

    inline const std::vector<vk::raii::DescriptorSet> &DescriptorSet::Get() const
    {
        return m_descriptorSets;
    }
}
