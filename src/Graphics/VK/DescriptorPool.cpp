#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/UniformBuffer.hpp>

using namespace Droplet::Graphics::VK;

DescriptorPool::DescriptorPool(const vk::raii::Device &p_device)
{
	std::array<vk::DescriptorPoolSize, 2> poolSize
	{
		{
			{
				.type = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = std::numeric_limits<std::uint8_t>::max()  
			},
			{
				.type = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = std::numeric_limits<std::uint8_t>::max()
			}
		} 
	};
	
	vk::DescriptorPoolCreateInfo poolInfo
	{ 
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
		.maxSets = static_cast<std::uint32_t>(poolSize.size()),
		.poolSizeCount = static_cast<std::uint32_t>(poolSize.size()),
		.pPoolSizes = poolSize.data() 
	};
	
	m_descriptorPool = vk::raii::DescriptorPool(p_device, poolInfo);
}
