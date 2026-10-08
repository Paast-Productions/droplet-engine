#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    enum class DescriptorSetBinding : std::uint32_t
    {
        eUniformBuffer = 0,
        eCombinedImageSampler = 1
    };
    
    class DescriptorSet
    {
    public:
        DescriptorSet() = delete;
        
        DescriptorSet(std::nullptr_t p_nullptr) :
            m_descriptorSet(p_nullptr),
            m_descriptorSetLayout(p_nullptr) {}
        
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool);
        
        DescriptorSet(const DescriptorSet &p_other) = delete;
        DescriptorSet &operator=(const DescriptorSet &p_other) = delete;
        
        DescriptorSet(DescriptorSet &&p_other) noexcept
        {
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            
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
            
            m_offset = p_other.m_offset;
            
            return *this;
        };

        ~DescriptorSet() noexcept = default;
        
        bool operator==(const DescriptorSet &p_other) const
        {
            return (m_descriptorSet == p_other.m_descriptorSet              &&
                    m_descriptorSetLayout == p_other.m_descriptorSetLayout  &&
                    m_offset == p_other.m_offset);
        }
        
        void AddBufferDescriptor(const vk::raii::Device &p_device, const vma::raii::Buffer &p_buffer);
        void AddSamplerDescriptor(const vk::raii::Device &p_device, const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView);
        
        const vk::raii::DescriptorSet &Get() const { return m_descriptorSet; }
        const vk::raii::DescriptorSetLayout &GetLayout() const { return m_descriptorSetLayout; }
        
    private:
        vk::raii::DescriptorSet m_descriptorSet { nullptr };
        vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
        
        std::vector<vk::ImageView> m_textures {};
        std::vector<vk::Buffer> m_buffers {};
        
        // ONLY USED IF vk::DescriptorType == eUniformBuffer
        // MAYBE REFACTOR?
        std::size_t m_offset {0};
     
    };
}
