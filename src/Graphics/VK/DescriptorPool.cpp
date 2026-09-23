#include <Graphics/VK/DescriptorPool.hpp>
#include <Graphics/VK/UniformBuffer.hpp>

using namespace Droplet::Graphics::VK;

DescriptorPool::DescriptorPool(vk::raii::Device const &p_device, std::uint32_t p_maxFramesInFlight)
{

}

void DescriptorPool::CreateDescriptorPool(vk::raii::Device const &p_device, std::uint32_t p_maxFramesInFlight)
{
	std::array<vk::DescriptorPoolSize, 2> poolSize{ {{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = p_maxFramesInFlight},
												{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = p_maxFramesInFlight}} };
	vk::DescriptorPoolCreateInfo          poolInfo{ .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
												   .maxSets = p_maxFramesInFlight,
												   .poolSizeCount = static_cast<std::uint32_t>(poolSize.size()),
												   .pPoolSizes = poolSize.data() };
	m_descriptorPool = vk::raii::DescriptorPool(p_device, poolInfo);

}

/*void DescriptorPool::CreateDescriptorSets(vk::raii::Device const &p_device, std::uint32_t p_maxFramesInFlight, std::optional<UniformBuffer> const &p_uniformBuffers)
{
	std::vector<vk::DescriptorSetLayout> layouts(p_maxFramesInFlight, m_descriptorSetLayout);
	vk::DescriptorSetAllocateInfo        allocInfo{
		.descriptorPool = m_descriptorPool,
		.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		.pSetLayouts = layouts.data() };

	m_descriptorSets.clear();
	m_descriptorSets = p_device.allocateDescriptorSets(allocInfo);

	for (size_t i = 0; i < p_maxFramesInFlight; i++)
	{
		vk::DescriptorBufferInfo bufferInfo{ .buffer = *p_uniformBuffers[i].value().GetBuffer(), .offset = 0, .range = sizeof(UniformBufferObject) };
		vk::DescriptorImageInfo  imageInfo{ .sampler = m_textureSampler, .imageView = m_textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal };

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{ {{.dstSet = m_descriptorSets[i],
																 .dstBinding = 0,
																 .dstArrayElement = 0,
																 .descriptorCount = 1,
																 .descriptorType = vk::DescriptorType::eUniformBuffer,
																 .pBufferInfo = &bufferInfo},
																{.dstSet = m_descriptorSets[i],
																 .dstBinding = 1,
																 .dstArrayElement = 0,
																 .descriptorCount = 1,
																 .descriptorType = vk::DescriptorType::eCombinedImageSampler,
																 .pImageInfo = &imageInfo}} };
		p_device.updateDescriptorSets(descriptorWrites, {});
	}
}*/

//defines shader stages, binding indices and descriptortype
void DescriptorPool::CreateDescriptorSetLayout(vk::raii::Device const &p_device)
{
	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
			{{.binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex},
			//Specify where the sampler is to be used with the ShaderStageFlag
			 {.binding = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment}} };
	vk::DescriptorSetLayoutCreateInfo layoutInfo{ .bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data() };
	m_descriptorSetLayout = vk::raii::DescriptorSetLayout(p_device, layoutInfo);
}

//adds descriptor info requried to create descriptorssetlayouts
void DescriptorPool::AddDescriptorBuffer(vk::raii::Buffer const &p_buffer, uint32_t p_byteSize)
{
	m_bufferDescs.push_back({ 
		.buffer = p_buffer, 
		.offset = 0, 
		.range = p_byteSize });
}

void DescriptorPool::AddDescriptorImage(vk::raii::Sampler const &p_sampler, vk::raii::ImageView const &p_imageView)
{
	m_imageDescs.push_back({ 
		.sampler = *p_sampler, 
		.imageView = *p_imageView, 
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal });
}