#include "DescriptorSetLayout.hpp"

#include <array>
#include <limits>

using namespace Droplet::Graphics::VK;

DescriptorSetLayout::DescriptorSetLayout(const vk::raii::Device& p_device)
{
    /*std::array<vk::DescriptorType, 3> types 
    {
        vk::DescriptorType::eUniformBuffer,
        vk::DescriptorType::eStorageBuffer,
        vk::DescriptorType::eCombinedImageSampler
    };*/
    
    std::array<vk::DescriptorType, 3> types {};
    types[static_cast<size_t>(BufferBindingType::UNIFORM)] = vk::DescriptorType::eUniformBuffer;
    types[static_cast<size_t>(BufferBindingType::STORAGE)] = vk::DescriptorType::eStorageBuffer;
    types[static_cast<size_t>(BufferBindingType::TEXTURE)] = vk::DescriptorType::eCombinedImageSampler;
    
    
    
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings {};
    std::array<vk::DescriptorBindingFlags, 3> flags {};
    for (std::uint8_t i {0}; i < 3; ++i)
    {
        bindings.at(i).binding = i;
        bindings.at(i).descriptorType = types.at(i);
        bindings.at(i).descriptorCount = std::numeric_limits<std::uint16_t>::max();
        bindings.at(i).stageFlags = vk::ShaderStageFlagBits::eAll;
        flags.at(i) = vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
    }
    
    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags
    {
        .sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo,
        .pNext = nullptr,
        .bindingCount = static_cast<std::uint32_t>(flags.size()),
        .pBindingFlags = flags.data()
    };
    
    vk::DescriptorSetLayoutCreateInfo createInfo
    {
        .sType = vk::StructureType::eDescriptorSetLayoutCreateInfo,
        .pNext = &bindingFlags,
        .flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool, 
        .bindingCount = static_cast<std::uint32_t>(bindings.size()),
        .pBindings = bindings.data()
    };
    
    m_descriptorSetLayout = {p_device, createInfo};
}
