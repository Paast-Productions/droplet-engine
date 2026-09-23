#include "BufferStorage.hpp"

using namespace Droplet::Graphics::VK;

BufferStorage::BufferStorage(const vk::raii::PhysicalDevice &p_physicalDevice)
{
    vk::PhysicalDeviceProperties properties { p_physicalDevice.getProperties() };
    
    m_minUniformBufferOffsetAlignment = static_cast<std::uint32_t>(properties.limits.minUniformBufferOffsetAlignment);
}

BufferHandle BufferStorage::StoreBuffer(const vk::raii::Device &p_device, const vk::raii::Buffer &p_buffer, vk::BufferUsageFlagBits p_usage)
{
    vk::DescriptorSet testSet {};
    
    std::uint32_t newHandle = static_cast<std::uint32_t>(m_buffers.size());
    m_buffers.push_back(p_buffer);
    
    std::array<vk::WriteDescriptorSet, 2> writes {};
    for (auto &write : writes)
    {
        vk::DescriptorBufferInfo bufferInfo 
        {
            .buffer = p_buffer,
            .offset = 0,
            .range = vk::WholeSize
        };
        
        write.dstSet = testSet;
        write.descriptorCount = 1;
        write.dstArrayElement = newHandle;
        write.pBufferInfo = &bufferInfo;
    }
    
    // I don't really get the point of the index thing :WhatTheHelly:
    std::uint32_t index {0};
    
    if ((p_usage & vk::BufferUsageFlagBits::eUniformBuffer) == vk::BufferUsageFlagBits::eUniformBuffer)
    {
        writes.at(index).dstBinding = static_cast<std::uint32_t>(BufferBindingType::UNIFORM);
        writes.at(index).descriptorType = vk::DescriptorType::eUniformBuffer;
        index++;
    }
    
    if ((p_usage & vk::BufferUsageFlagBits::eStorageBuffer) == vk::BufferUsageFlagBits::eStorageBuffer)
    {
        writes.at(index).dstBinding = static_cast<std::uint32_t>(BufferBindingType::STORAGE);
        writes.at(index).descriptorType = vk::DescriptorType::eStorageBuffer;
    }
    
    p_device.updateDescriptorSets(writes, nullptr);
    
    return static_cast<BufferHandle>(newHandle);
}
