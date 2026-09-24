#include "BufferHelper.hpp"

#include "CommandPool.hpp"
#include "CommandBuffer.hpp"

uint32_t FindMemoryType(const vk::raii::PhysicalDevice &p_physDevice, const uint32_t p_typeFilter, const vk::MemoryPropertyFlags p_properties)
{
	const vk::PhysicalDeviceMemoryProperties memProperties = p_physDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((p_typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physDevice, const vk::DeviceSize p_size, const vk::BufferUsageFlags p_usage, const vk::MemoryPropertyFlags p_properties)
{
	vk::BufferCreateInfo   bufferInfo
	{
		.size = p_size, 
		.usage = p_usage,
		.sharingMode = vk::SharingMode::eExclusive
	};
	
	vk::raii::Buffer       buffer { vk::raii::Buffer(p_device, bufferInfo) };
	const vk::MemoryRequirements memRequirements { buffer.getMemoryRequirements() };
	vk::MemoryAllocateInfo allocInfo
	{
		.allocationSize = memRequirements.size, 
		.memoryTypeIndex = FindMemoryType(p_physDevice, memRequirements.memoryTypeBits, p_properties)
	};
	
	vk::raii::DeviceMemory bufferMemory { vk::raii::DeviceMemory(p_device, allocInfo) };
	buffer.bindMemory(*bufferMemory, 0);

	//vk::raii handles can't be copied so they have to be moved
	return { std::move(buffer), std::move(bufferMemory) };
}

void CopyBuffer(const vk::raii::Queue &p_queue, const Droplet::Graphics::VK::CommandPool &p_commandPool, const vk::raii::Buffer &p_srcBuffer, const vk::raii::Buffer &p_dstBuffer, vk::DeviceSize p_size)
{
	p_commandPool.ImmediateSubmit(p_queue,
	[&p_srcBuffer, &p_dstBuffer, p_size](Droplet::Graphics::VK::CommandBuffer& p_commandBuffer) // A command buffer is the parameter
	{
		p_commandBuffer.CopyBuffer(*p_srcBuffer, *p_dstBuffer, p_size); // What to perform on the command buffer
	});
}

void TransitionImageLayout(vk::raii::CommandBuffer &p_commandBuffer, const vk::raii::Image &p_image, vk::ImageLayout p_oldLayout, vk::ImageLayout p_newLayout)
{
	vk::ImageMemoryBarrier barrier{ .oldLayout = p_oldLayout,
								   .newLayout = p_newLayout,
								   .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
								   .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
								   .image = p_image,
								   .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1} };

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (p_oldLayout == vk::ImageLayout::eUndefined && p_newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (p_oldLayout == vk::ImageLayout::eTransferDstOptimal && p_newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}
	p_commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, {}, barrier);
}