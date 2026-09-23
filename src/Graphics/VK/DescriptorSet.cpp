#include "DescriptorSet.hpp"

using namespace Droplet::Graphics::VK;

DescriptorSet::DescriptorSet(const vk::raii::Device &p_device, const vk::raii::DescriptorPool &p_descriptorPool, const vk::raii::DescriptorSetLayout &p_descriptorSetLayout)
{
    vk::DescriptorSetAllocateInfo allocateInfo
    {
        .sType = vk::StructureType::eDescriptorSetAllocateInfo,
        .pNext = nullptr,
        .descriptorPool = p_descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = reinterpret_cast<vk::DescriptorSetLayout const *>(&p_descriptorSetLayout),
    };
    
    m_descriptorSets = p_device.allocateDescriptorSets(allocateInfo);
}
