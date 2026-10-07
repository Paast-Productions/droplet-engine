#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    class DescriptorSet
    {
    public:
        DescriptorSet() = delete;
        
        DescriptorSet(std::nullptr_t p_nullptr) :
            m_descriptorSet(p_nullptr),
            m_descriptorSetLayout(p_nullptr) {}
        
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool, vk::DescriptorType p_descType);
        
        DescriptorSet(const DescriptorSet &p_other) = delete;
        DescriptorSet &operator=(const DescriptorSet &p_other) = delete;
        
        DescriptorSet(DescriptorSet &&p_other) noexcept
        {
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            std::swap(m_descriptorsToWrite, p_other.m_descriptorsToWrite);
            
            m_descriptorType = p_other.m_descriptorType;
            m_offset = p_other.m_offset;
        };
        
        DescriptorSet &operator=(DescriptorSet &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            std::swap(m_descriptorsToWrite, p_other.m_descriptorsToWrite);
            
            m_descriptorType = p_other.m_descriptorType;
            m_offset = p_other.m_offset;
            
            return *this;
        };

        ~DescriptorSet() noexcept = default;
        
        bool operator==(const DescriptorSet &p_other) const
        {
            return (m_descriptorSet == p_other.m_descriptorSet              &&
                    m_descriptorSetLayout == p_other.m_descriptorSetLayout  &&
                    m_descriptorType == p_other.m_descriptorType            &&
                    m_descriptorsToWrite == p_other.m_descriptorsToWrite    &&
                    m_offset == p_other.m_offset);
        }
        
        // TYPE == eUniformBuffer
        void AddBufferDescriptor(const vma::raii::Buffer &p_buffer);
        
        // TYPE == eCombinedImageSampler
        void AddSamplerDescriptor(const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView);
        
        void WriteDescriptors(const vk::raii::Device &p_device) const;
        
        const vk::raii::DescriptorSet &Get() const { return m_descriptorSet; }
        const vk::raii::DescriptorSetLayout &GetLayout() const { return m_descriptorSetLayout; }
        const vk::DescriptorType &GetType() const { return m_descriptorType; }
        
    private:
        vk::raii::DescriptorSet m_descriptorSet { nullptr };
        vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
        vk::DescriptorType m_descriptorType {};
        
        std::vector<vk::WriteDescriptorSet> m_descriptorsToWrite {};
        
        // ONLY USED IF vk::DescriptorType == eUniformBuffer
        // MAYBE REFACTOR?
        std::size_t m_offset {0};
        
        std::uint8_t m_bindingNum {0};
        inline static std::uint8_t s_numDescriptorSets {0};
        
    };
}
