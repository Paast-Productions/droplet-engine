#include "DescriptorPool.hpp"

using namespace Droplet::Graphics::VK;

DescriptorPool::DescriptorPool(const vk::raii::Device &p_device, const std::vector<vk::DescriptorPoolSize> &p_poolSizes)
{
	
	vk::DescriptorPoolCreateInfo poolInfo
	{ 
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
		.maxSets = static_cast<std::uint32_t>(p_poolSizes.size()),
		.poolSizeCount = static_cast<std::uint32_t>(p_poolSizes.size()),
		.pPoolSizes = p_poolSizes.data() 
	};
	
	m_descriptorPool = vk::raii::DescriptorPool(p_device, poolInfo);
}
