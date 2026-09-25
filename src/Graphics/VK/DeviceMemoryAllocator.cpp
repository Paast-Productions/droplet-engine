#include "DeviceMemoryAllocator.hpp"

#include <memory>

using namespace Droplet::Graphics::VK;

template <class TData>
std::uint32_t DeviceMemoryAllocator::AddRange(TData &&p_data)
{
    std::size_t dataSize { sizeof TData };
    std::unique_ptr<TData> bytes { std::make_unique<TData>(p_data) };
    
    std::uint32_t currentOffset { m_lastOffset };
    m_ranges.push_back({currentOffset, static_cast<std::uint32_t>(dataSize), bytes});
    
    m_lastOffset += PadSizeToMinAlignment(static_cast<std::uint32_t>(dataSize));
    return currentOffset;
}

void DeviceMemoryAllocator::Build(const vk::raii::Device &p_device, const vma::raii::Allocator &p_allocator, const vk::raii::DescriptorPool &p_descriptorPool)
{
    vk::BufferCreateInfo bufferCreateInfo
    {
        .size = static_cast<std::size_t>(m_lastOffset),
        .usage = vk::BufferUsageFlagBits::eUniformBuffer
    };
    
    vma::AllocationCreateInfo allocationCreateInfo
    {
        .usage = vma::MemoryUsage::eAuto
    };
    
    vma::raii::Buffer buffer {p_allocator.createBuffer(bufferCreateInfo, allocationCreateInfo)};
    const vma::raii::Allocation& allocation = buffer.getAllocation();
    
    // CRITICAL COPY AREA <START>
    void* data { allocation.map() };
    
    for (const auto &range : m_ranges)
    {
        std::memcpy(data + range.offset, std::any_cast<void *>(range.bytes), range.size);
    }
    
    allocation.unmap();
    // CRITICAL COPY AREA <END>
    
    vk::DescriptorSetLayoutBinding binding 
    {
        .binding = 1,
        .descriptorType = vk::DescriptorType::eUniformBufferDynamic,
        .descriptorCount = 1,
        .stageFlags = vk::ShaderStageFlagBits::eAll,
    };
    
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo 
    {  
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .bindingCount = 1,
        .pBindings = &binding,
    };
    
    vk::DescriptorSetLayout descriptorSetLayout {p_device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo)};
    
    
    std::uint32_t maxRangeSize {0};
    for (auto &range : m_ranges )
    {
        maxRangeSize = std::max(range.size, maxRangeSize);
    }
 
    vk::DescriptorSetAllocateInfo allocateInfo 
    {
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = nullptr,
        .descriptorPool = p_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &m_descriptorSetLayout
    };
 
    vk::raii::DescriptorSet a { p_device.allocateDescriptorSets(allocateInfo).at(0)};   
}
