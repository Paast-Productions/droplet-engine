#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
    /// @enum DescriptorSetIndex
    /// @brief Enum that describes the binding index for each type of descriptor
    enum class DescriptorSetIndex : std::uint32_t
    {
        eUniformBuffer = 0,
        eStorageBuffer = 1,
        eCombinedImageSampler = 2
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
        /// @param p_descriptorType Vulkan Descriptor Type
        DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool, vk::DescriptorType p_descriptorType);
        
        DescriptorSet(const DescriptorSet &p_other) = delete;
        DescriptorSet &operator=(const DescriptorSet &p_other) = delete;
        
        DescriptorSet(DescriptorSet &&p_other) noexcept
        {
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            m_descriptorType = p_other.m_descriptorType;
        };
        
        DescriptorSet &operator=(DescriptorSet &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_descriptorSet, p_other.m_descriptorSet);
            std::swap(m_descriptorSetLayout, p_other.m_descriptorSetLayout);
            m_descriptorType = p_other.m_descriptorType;
            
            return *this;
        };

        ~DescriptorSet() noexcept = default;
        
        bool operator==(const DescriptorSet &p_other) const
        {
            return (m_descriptorSet == p_other.m_descriptorSet              &&
                    m_descriptorSetLayout == p_other.m_descriptorSetLayout  &&
                    m_descriptorType == p_other.m_descriptorType);
        }
        
        /// @brief Writes a buffer descriptor to the descriptor set
        /// @param p_device Vulkan Device
        /// @param p_buffer VMA Buffer
        /// @param p_dstIndex Destination index of descriptor set
        /// @throws std::runtime_error If descriptor type of descriptor set is not a buffer
        void WriteDescriptor(const vk::raii::Device &p_device, const vma::raii::Buffer &p_buffer, std::uint32_t p_dstIndex) const;
        
        /// @brief Writes a texture descriptor to the descriptor set 
        /// @param p_device Vulkan Device
        /// @param p_sampler Vulkan Sampler
        /// @param p_imageView Vulkan ImageView
        /// @param p_dstIndex Destination index of descriptor set
        /// @throws std::runtime_error If descriptor type of descriptor set is not a texture
        void WriteDescriptor(const vk::raii::Device &p_device, const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView, uint32_t
                             p_dstIndex) const;
        
        /// @brief Getter-function for the descriptor set
        /// @return Vulkan DescriptorSet
        [[nodiscard]] const vk::raii::DescriptorSet &Get() const { return m_descriptorSet; }
        
        /// @brief Getter-function for the descriptor set layout
        /// @returns Vulkan DescriptorSetLayout
        [[nodiscard]] const vk::raii::DescriptorSetLayout &GetLayout() const { return m_descriptorSetLayout; }
        
    private:
        vk::raii::DescriptorSet m_descriptorSet { nullptr };
        vk::raii::DescriptorSetLayout m_descriptorSetLayout { nullptr };
        vk::DescriptorType m_descriptorType {};
        
        inline const static std::vector<vk::DescriptorType> s_bufferTypes 
        {
            vk::DescriptorType::eUniformBuffer,
            vk::DescriptorType::eStorageBuffer
        };
        
        inline const static std::vector<vk::DescriptorType> s_textureTypes 
        {
            vk::DescriptorType::eCombinedImageSampler  
        };
        
    };
}
