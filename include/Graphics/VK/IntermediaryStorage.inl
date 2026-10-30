#pragma once

#include "IntermediaryStorage.hpp"

using namespace Droplet::Graphics::VK;

template <typename T>
std::uint32_t IntermediaryStorage::CreateBufferResource(const T &p_data, vk::DescriptorType p_type)
{
    switch (p_type)
    {
    case vk::DescriptorType::eUniformBuffer:
    {
        std::uint32_t dstIndex { static_cast<std::uint32_t>(m_uniformBuffers.size()) };
        m_uniformBuffers.emplace_back(m_allocator.Get(), p_data);
        
        VK::DescriptorSet &descriptorSet = m_descriptorSets.at(m_descriptorSetIndexMap.at(p_type));
        descriptorSet.WriteDescriptor(m_context->GetDevice(), m_uniformBuffers.at(dstIndex).Get(), dstIndex);    
        
        return dstIndex;
    }
        
    default:
        throw std::runtime_error("Descriptor Type not implemented");
    }
    
}

template <typename T>
void IntermediaryStorage::UpdateBufferResourceAt(std::uint32_t p_index, const T &p_data, vk::DescriptorType p_type)
{
    switch (p_type)
    {
    case vk::DescriptorType::eUniformBuffer:
        m_uniformBuffers.at(p_index).UpdateBuffer(p_data);
        
        return;
        
    default:
        throw std::runtime_error("Descriptor Type not implemented");
    }
    
}
