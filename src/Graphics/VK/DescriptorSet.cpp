#include "DescriptorSet.hpp"

#include <print>
#include <iostream>
#include <algorithm>

using namespace Droplet::Graphics::VK;

DescriptorSet::DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool, vk::DescriptorType p_descriptorType) : 
    m_descriptorType (p_descriptorType)
{
    constexpr vk::DescriptorBindingFlags bindingFlags
    {
        vk::DescriptorBindingFlagBits::ePartiallyBound 
        | vk::DescriptorBindingFlagBits::eUpdateAfterBind
        | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
        | vk::DescriptorBindingFlagBits::eVariableDescriptorCount
    };
    
    const vk::DescriptorSetLayoutBinding binding 
    {
        .binding = 0,
        .descriptorType = m_descriptorType,
        .descriptorCount = std::numeric_limits<std::uint8_t>::max(), 
        .stageFlags = vk::ShaderStageFlagBits::eAll
    };
    
    const vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingCreateInfo
    {
        .sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo,
        .bindingCount = 1,
        .pBindingFlags = &bindingFlags,
    };
    
    const vk::DescriptorSetLayoutCreateInfo createInfo 
    {
        .sType = vk::StructureType::eDescriptorSetLayoutCreateInfo,
        .pNext = &bindingCreateInfo,
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
        .bindingCount = 1,
        .pBindings = &binding
    };

    try
    {
        m_descriptorSetLayout = p_device.createDescriptorSetLayout(createInfo);
    }
    catch (const std::exception &err)
    {
        std::print("[ERROR: DescriptorSetLayout]: {0}", err.what());
    }
    
    std::vector<std::uint32_t> descriptorCounts 
    {
        std::numeric_limits<std::uint8_t>::max()
    };
    
    vk::StructureChain<vk::DescriptorSetVariableDescriptorCountAllocateInfo> descriptorInfoChain 
    {
         {  // vk::DescriptorSetVariableDescriptorCountAllocateInfo
             .descriptorSetCount = 1,
             .pDescriptorCounts = descriptorCounts.data() 
         }
    };
    
    vk::DescriptorSetAllocateInfo allocateInfo
    {
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = descriptorInfoChain.get<vk::DescriptorSetVariableDescriptorCountAllocateInfo>(),
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

void DescriptorSet::WriteDescriptor(const vk::raii::Device &p_device, const vma::raii::Buffer &p_buffer, const std::uint32_t p_dstIndex) const
{
    if (std::ranges::find(s_bufferTypes, m_descriptorType) == s_bufferTypes.end())
    {
        throw std::runtime_error("[[DescriptorSet::WriteDescriptor]] Tried to write texture to buffer descriptor set");
    }
    
    vk::DescriptorBufferInfo bufferInfo
    {
        .buffer = p_buffer,
        .offset = 0,
        .range = vk::WholeSize,
    };
   
    const vk::WriteDescriptorSet descriptorWrite
    {
        .dstSet = m_descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = p_dstIndex,
        .descriptorCount = 1,
        .descriptorType = m_descriptorType,
        .pBufferInfo = &bufferInfo
    };
    
    p_device.updateDescriptorSets(descriptorWrite, {});
}

void DescriptorSet::WriteDescriptor(const vk::raii::Device &p_device, const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView, const uint32_t p_dstIndex) const
{
    if (std::ranges::find(s_textureTypes, m_descriptorType) == s_textureTypes.end())
    {
        throw std::runtime_error("[[DescriptorSet::WriteDescriptor]] Tried to write buffer to texture descriptor set");
    }
    
    vk::DescriptorImageInfo imageInfo
    {
        .sampler = p_sampler,
        .imageView = p_imageView,
        .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
    };
    
    const vk::WriteDescriptorSet descriptorWrite
    {
        .dstSet = m_descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = p_dstIndex, 
        .descriptorCount = 1,
        .descriptorType = m_descriptorType,
        .pImageInfo = &imageInfo
    };
    
    p_device.updateDescriptorSets(descriptorWrite, {});
}
