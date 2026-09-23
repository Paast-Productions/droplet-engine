#include "BufferHelper.hpp"


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

vk::raii::CommandBuffer BeginSingleTimeCommands(vk::raii::Device const &p_device, vk::raii::CommandPool const &p_commandPool)
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = p_commandPool, 
		.level = vk::CommandBufferLevel::ePrimary, 
		.commandBufferCount = 1
	};
	
	vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(p_device, allocInfo).front());

	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommands(vk::raii::CommandBuffer &&p_commandBuffer, vk::raii::Queue const &p_queue)
{
	p_commandBuffer.end();

	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*p_commandBuffer };
	p_queue.submit(submitInfo, nullptr);
	p_queue.waitIdle();
}

void CopyBuffer(const vk::raii::Device &p_device, const vk::raii::Queue &p_queue, const vk::raii::CommandPool &p_commandPool, const vk::raii::Buffer &p_srcBuffer, const vk::raii::Buffer &p_dstBuffer, const vk::DeviceSize p_size)
{
	vk::raii::CommandBuffer commandCopyBuffer = BeginSingleTimeCommands(p_device, p_commandPool);
	commandCopyBuffer.copyBuffer(*p_srcBuffer, *p_dstBuffer, vk::BufferCopy{ .size = p_size });
	EndSingleTimeCommands(std::move(commandCopyBuffer), p_queue);
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