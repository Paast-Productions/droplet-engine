#include "CommandPool.hpp"

using namespace Droplet::Graphics::Vulkan;

CommandPool::CommandPool(
	const vk::raii::Device &p_device, 
	const std::uint32_t p_queueFamilyIndex,
	const vk::CommandPoolCreateFlags p_flags,
	const std::uint32_t p_count)
{
	vk::CommandPoolCreateInfo createInfo 
	{
		.flags = p_flags, 
		.queueFamilyIndex = p_queueFamilyIndex
	};
	
	try
	{
		m_commandPool = vk::raii::CommandPool {p_device, createInfo};
	} 
	catch (const vk::Error &err)
	{
		std::print(std::cerr, "CommandPool-CommandPool Error: {0}", err.what());
		std::flush(std::cerr);
	}
	
	vk::CommandBufferAllocateInfo bufferAllocInfo
	{
		.commandPool = m_commandPool,
		.level = vk::CommandBufferLevel::ePrimary, 
		.commandBufferCount = p_count
	};
	
	try
	{
		m_commandBuffers = vk::raii::CommandBuffers{p_device, bufferAllocInfo};
	} 
	catch (const vk::Error &err)
	{
		std::print(std::cerr, "CommandPool-CommandBuffers Error: {0}", err.what());
		std::flush(std::cerr);
	}
}

std::size_t Droplet::Graphics::Vulkan::CommandPool::Allocate(const vk::raii::Device &p_device, const vk::CommandBufferLevel p_level)
{
	vk::CommandBufferAllocateInfo allocInfo
	{
		.commandPool = m_commandPool,
		.level = p_level, 
		.commandBufferCount = 1
	};
	 
	try
	{
		m_commandBuffers.push_back(std::move(vk::raii::CommandBuffers{ p_device, allocInfo }.front()));
	} 
	catch (const vk::Error &err)
	{
		std::print(std::cerr, "ComandPool-CommandBuffer Error: {0}", err.what());
		std::flush(std::cerr);
	}
	return m_commandBuffers.size() - 1;
}

vk::raii::CommandBuffer &CommandPool::GetBufferAt(const std::size_t p_id)
{
	assert(p_id < m_commandBuffers.size());
	return m_commandBuffers.at(p_id);
}
