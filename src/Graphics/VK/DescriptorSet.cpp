#include "DescriptorSet.hpp"

#include <print>

using namespace Droplet::Graphics::VK;

DescriptorSet::DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool) 
{
    constexpr std::array<vk::DescriptorBindingFlags, 2> bindingFlags
    {
        {
            {
               vk::DescriptorBindingFlagBits::ePartiallyBound 
               | vk::DescriptorBindingFlagBits::eUpdateAfterBind
               | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
            },
            {vk::DescriptorBindingFlagBits::ePartiallyBound 
            | vk::DescriptorBindingFlagBits::eUpdateAfterBind
            | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
            }
        }
    };
    
    constexpr std::array<vk::DescriptorSetLayoutBinding, 2> bindings 
    {
        {
            {
                .binding = static_cast<std::uint32_t>(DescriptorSetBinding::eUniformBuffer),
                .descriptorType = vk::DescriptorType::eUniformBuffer,
                .descriptorCount = std::numeric_limits<std::uint8_t>::max(), 
                .stageFlags = vk::ShaderStageFlagBits::eAll
            },
            {
                .binding = static_cast<std::uint32_t>(DescriptorSetBinding::eCombinedImageSampler),
                .descriptorType = vk::DescriptorType::eCombinedImageSampler,
                .descriptorCount = std::numeric_limits<std::uint8_t>::max(), 
                .stageFlags = vk::ShaderStageFlagBits::eAll
            }
        }    
    };
    
    const vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingCreateInfo
    {
        .sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo,
        .bindingCount = static_cast<std::uint32_t>(bindingFlags.size()),
        .pBindingFlags = bindingFlags.data(),
    };
    
    const vk::DescriptorSetLayoutCreateInfo createInfo 
    {
        .sType = vk::StructureType::eDescriptorSetLayoutCreateInfo,
        .pNext = &bindingCreateInfo,
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
        .bindingCount = static_cast<std::uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };

    try
    {
        m_descriptorSetLayout = p_device.createDescriptorSetLayout(createInfo);
    }
    catch (const std::exception &err)
    {
        std::print("[ERROR: DescriptorSetLayout]: {0}", err.what());
    }
  
    const vk::DescriptorSetAllocateInfo allocateInfo
    {
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = nullptr,
        .descriptorPool = p_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &*m_descriptorSetLayout,
    };
    
    try
    {
        m_descriptorSet = std::move(p_device.allocateDescriptorSets(allocateInfo).at(0));
    }
    catch (const std::exception &err)
    {
        std::print("[ERROR: DescriptorSet]: {0}", err.what());
    }
}

void DescriptorSet::WriteUniformBufferDescriptor(const vk::raii::Device &p_device, const vma::raii::Buffer &p_buffer)
{
    vk::DescriptorBufferInfo bufferInfo
    {
        .buffer = p_buffer,
        .offset = 0,
        .range = vk::WholeSize,
    };
   
    const vk::WriteDescriptorSet descriptorWrite
    {
        .dstSet = m_descriptorSet,
        .dstBinding = static_cast<std::uint32_t>(DescriptorSetBinding::eUniformBuffer),
        .dstArrayElement = static_cast<std::uint32_t>(m_buffers.size()),
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
        .pBufferInfo = &bufferInfo
    };
    
    m_buffers.emplace_back(*p_buffer);
    p_device.updateDescriptorSets(descriptorWrite, {});
}

void DescriptorSet::WriteCombinedImageSamplerDescriptor(const vk::raii::Device &p_device, const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView)
{
    vk::DescriptorImageInfo imageInfo
    {
        .sampler = p_sampler,
        .imageView = p_imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    
    const vk::WriteDescriptorSet descriptorWrite
    {
        .dstSet = m_descriptorSet,
        .dstBinding = static_cast<std::uint32_t>(DescriptorSetBinding::eCombinedImageSampler),
        .dstArrayElement = static_cast<std::uint32_t>(m_textures.size()), 
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eCombinedImageSampler,
        .pImageInfo = &imageInfo
    };
    
    m_textures.emplace_back(p_imageView);
    p_device.updateDescriptorSets(descriptorWrite, {});
}
