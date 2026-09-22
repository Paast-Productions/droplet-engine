#include "BufferHelper.hpp"


uint32_t FindMemoryType(vk::raii::PhysicalDevice p_physDevice, uint32_t p_typeFilter, vk::MemoryPropertyFlags p_properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = p_physDevice.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((p_typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & p_properties) == p_properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(vk::raii::Device const &p_device, vk::raii::PhysicalDevice const &p_physDevice, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage, vk::MemoryPropertyFlags p_properties)
{
	vk::BufferCreateInfo   bufferInfo{ .size = p_size, .usage = p_usage, .sharingMode = vk::SharingMode::eExclusive };
	vk::raii::Buffer       buffer = vk::raii::Buffer(p_device, bufferInfo);
	vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo{ .allocationSize = memRequirements.size, .memoryTypeIndex = FindMemoryType(p_physDevice,memRequirements.memoryTypeBits, p_properties) };
	vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(p_device, allocInfo);
	buffer.bindMemory(*bufferMemory, 0);

	//vk::raii handles can't be copied so they have to be moved
	return { std::move(buffer), std::move(bufferMemory) };
}

vk::raii::CommandBuffer BeginSingleTimeCommands(vk::raii::Device const &p_device, vk::raii::CommandPool const &p_commandPool)
{
	vk::CommandBufferAllocateInfo allocInfo{ .commandPool = p_commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1 };
	vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(p_device, allocInfo).front());

	vk::CommandBufferBeginInfo beginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
	commandBuffer.begin(beginInfo);

	return std::move(commandBuffer);
}

void EndSingleTimeCommands(vk::raii::CommandBuffer &&p_commandBuffer, vk::raii::Queue const &p_queue)
{
	p_commandBuffer.end();

	vk::SubmitInfo submitInfo{ .commandBufferCount = 1, .pCommandBuffers = &*p_commandBuffer };
	p_queue.submit(submitInfo, nullptr);
	p_queue.waitIdle();
}

void CopyBuffer(vk::raii::Device const &p_device, vk::raii::Queue const &p_queue, vk::raii::CommandPool const &p_commandPool, vk::raii::Buffer &p_srcBuffer, vk::raii::Buffer &p_dstBuffer, vk::DeviceSize p_size)
{
	vk::raii::CommandBuffer commandCopyBuffer = BeginSingleTimeCommands(p_device, p_commandPool);
	commandCopyBuffer.copyBuffer(*p_srcBuffer, *p_dstBuffer, vk::BufferCopy{ .size = p_size });
	EndSingleTimeCommands(std::move(commandCopyBuffer), p_queue);
}