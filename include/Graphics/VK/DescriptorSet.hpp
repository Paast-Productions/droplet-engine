#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    /// @enum DescriptorSetBinding
    /// @brief Enum that describes the binding index for each type of descriptor
    enum class DescriptorSetBinding : std::uint32_t
    {
        eUniformBuffer = 0,
        eCombinedImageSampler = 1
    };
    
    /// @class DescriptorSet
    /// @brief Wrapper class for Vulkan DescriptorSet. This class contains its own layout and set handles.
    /// It is responsible for adding descriptors to itself that then get appended to the pipeline. This results
    /// in resources being accessible in shaders.
    class DescriptorSet
    {
    public:
        DescriptorSet() = delete;
        
        /// @brief DescriptorSet Nullptr Constructor
        /// @param p_nullptr nullptr
        DescriptorSet(std::nullptr_t p_nullptr) :
            m_descriptorSet(p_nullptr),
            m_descriptorSetLayout(p_nullptr) {}
        
        /// @brief DescriptorSet RAII Constructor
        /// @param p_device Vulkan Device
        /// @param p_descriptorPool Vulkan Descriptor Pool
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool);
        
        DescriptorSet(const DescriptorSet &p_other) = delete;
        DescriptorSet &operator=(const DescriptorSet &p_other) = delete;
        
        DescriptorSet(DescriptorSet &&p_other) noexcept
        {
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
        };
        
        DescriptorSet &operator=(DescriptorSet &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            
            return *this;
        };

        ~DescriptorSet() noexcept = default;
        
        bool operator==(const DescriptorSet &p_other) const
        {
            return (m_descriptorSet == p_other.m_descriptorSet              &&
                    m_descriptorSetLayout == p_other.m_descriptorSetLayout);
        }
        
        /// @brief Writes a uniform buffer descriptor to the uniform buffer binding in the descriptor set
        /// @param p_device Vulkan Device
        /// @param p_buffer VMA Buffer
        void WriteUniformBufferDescriptor(const vk::raii::Device &p_device, const vma::raii::Buffer &p_buffer);
        
        /// @brief Writes a combined image sampler descriptor to the combined image sampler binding in the descriptor set
        /// @param p_device Vulkan Device
        /// @param p_sampler Vulkan Sampler
        /// @param p_imageView Vulkan ImageView
        void WriteCombinedImageSamplerDescriptor(const vk::raii::Device &p_device, const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView);
        
        /// @brief Getter-function for the descriptor set
        /// @return Vulkan DescriptorSet
        [[nodiscard]] const vk::raii::DescriptorSet &Get() const { return m_descriptorSet; }
        
        /// @brief Getter-function for the descriptor set layout
        /// @returns Vulkan DescriptorSetLayout
        [[nodiscard]] const vk::raii::DescriptorSetLayout &GetLayout() const { return m_descriptorSetLayout; }
        
    private:
        vk::raii::DescriptorSet m_descriptorSet { nullptr };
        vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
        
        std::vector<vk::ImageView> m_textures {};
        std::vector<vk::Buffer> m_buffers {};
    };
}
