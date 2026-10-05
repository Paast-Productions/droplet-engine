#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = delete;
        
        /// @brief Descriptor Set Constructor
        /// @param p_device Vulkan Device
        /// @param p_descriptorPool Vulkan Descriptor Pool
        /// @param p_descriptorSetLayout Vulkan Descriptor Set Layout
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool& p_descriptorPool, const vk::raii::DescriptorSetLayout& p_descriptorSetLayout);
        
        DescriptorSet(const DescriptorSet &) = delete;
        DescriptorSet &operator=(const DescriptorSet &) = delete;
        DescriptorSet(DescriptorSet &&) = delete;
        DescriptorSet &operator=(DescriptorSet &&) = delete;

        ~DescriptorSet() = default;
        
        /// @brief Getter-function for the Vulkan Descriptor Set
        /// @returns Vulkan Descriptor Set
        [[nodiscard]] const std::vector<vk::raii::DescriptorSet> &Get() const;
        
    private:
        std::vector<vk::raii::DescriptorSet> m_descriptorSets {};
        
    };

    inline const std::vector<vk::raii::DescriptorSet> &DescriptorSet::Get() const
    {
        return m_descriptorSets;
    }
}
