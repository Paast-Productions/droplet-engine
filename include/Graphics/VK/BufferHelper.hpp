#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vector>

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

uint32_t FindMemoryType
	(vk::raii::PhysicalDevice p_physDevice, uint32_t p_typeFilter, vk::MemoryPropertyFlags p_properties);
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer
	(vk::raii::Device const &p_device, vk::raii::PhysicalDevice const &p_physDevice, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage, vk::MemoryPropertyFlags p_properties);
vk::raii::CommandBuffer BeginSingleTimeCommands
	(vk::raii::Device const &p_device, vk::raii::CommandPool const &p_commandPool);
void EndSingleTimeCommands
	(vk::raii::CommandBuffer &&p_commandBuffer, vk::raii::Queue const &p_queue);
void CopyBuffer
	(vk::raii::Device const &p_device, vk::raii::Queue const &p_queue, vk::raii::CommandPool const &p_commandPool, vk::raii::Buffer &p_srcBuffer, vk::raii::Buffer &p_dstBuffer, vk::DeviceSize p_size);