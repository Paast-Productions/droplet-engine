#include "DescriptorSet.hpp"

#include <print>

using namespace Droplet::Graphics::VK;

DescriptorSet::DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool, const vk::DescriptorType p_descType) : 
    m_descriptorType { p_descType }, 
    m_bindingNum { s_numDescriptorSets++ }
{
    constexpr vk::DescriptorBindingFlags bindingFlags
    {
        vk::DescriptorBindingFlagBits::eVariableDescriptorCount
        | vk::DescriptorBindingFlagBits::ePartiallyBound 
        | vk::DescriptorBindingFlagBits::eUpdateAfterBind
        | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
    };
    
    const vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingCreateInfo
    {
        .sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo,
        .bindingCount = 1,
        .pBindingFlags = &bindingFlags,
    };
    
    const vk::DescriptorSetLayoutBinding binding
    {
        .binding = m_bindingNum,
        .descriptorType = p_descType,
        .descriptorCount = std::numeric_limits<std::uint8_t>::max(), 
        .stageFlags = vk::ShaderStageFlagBits::eAll
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
  
    std::uint32_t maxDescriptors = std::numeric_limits<std::uint8_t>::max();
    
    vk::DescriptorSetVariableDescriptorCountAllocateInfo variableAllocateInfo
    {
        .descriptorSetCount = 1,
        .pDescriptorCounts = &maxDescriptors
    };
    
    const vk::DescriptorSetAllocateInfo allocateInfo
    {
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = &variableAllocateInfo,
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

void DescriptorSet::AddBufferDescriptor(const vma::raii::Buffer &p_buffer)
{
    if (m_descriptorType != vk::DescriptorType::eUniformBuffer)
    {
        throw std::runtime_error("[ERROR: AddBufferDescriptor]: Type of Descriptor Set is not eUniformBuffer");
    }
    
    const std::size_t range = p_buffer.getAllocation().getInfo2().allocationInfo.size; 
    
    vk::DescriptorBufferInfo bufferInfo
    {
        .buffer = p_buffer,
        .offset = m_offset,
        .range = range,
    };
    
    const vk::WriteDescriptorSet descriptorWrite
    {
        .dstSet = m_descriptorSet,
        .dstBinding = static_cast<std::uint32_t>(m_descriptorsToWrite.size()),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = m_descriptorType,
        .pBufferInfo = &bufferInfo
    };
    
    m_descriptorsToWrite.emplace_back(descriptorWrite);
    m_offset += range;
}

void DescriptorSet::AddSamplerDescriptor(const vk::raii::Sampler &p_sampler, const vk::raii::ImageView &p_imageView)
{
    if (m_descriptorType != vk::DescriptorType::eCombinedImageSampler)
    {
        throw std::runtime_error("[ERROR: AddSamplerDescriptor]: Type of Descriptor Set is not eCombinedImageSampler");
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
        .dstBinding = static_cast<std::uint32_t>(m_descriptorsToWrite.size()),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = m_descriptorType,
        .pImageInfo = &imageInfo
    };
    
    m_descriptorsToWrite.emplace_back(descriptorWrite);
}

void DescriptorSet::WriteDescriptors(const vk::raii::Device &p_device) const
{
    p_device.updateDescriptorSets(m_descriptorsToWrite, {});
}
