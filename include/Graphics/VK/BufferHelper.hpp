#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vector>

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

/// @brief Helper function to find memory properties of the hardware
/// @param p_physDevice RAII pointer reference to hardware device
/// @param p_typefilter Type bits of the hardwares memory requirements
/// @param p_properties Memory property flags
/// @returns memory type index
uint32_t FindMemoryType
	(vk::raii::PhysicalDevice p_physDevice, uint32_t p_typeFilter, vk::MemoryPropertyFlags p_properties);

/// @brief Helper function to allocate and create a buffer
/// @param p_device RAII pointer reference to the device
/// @param p_physDevice RAII pointer reference to hardware device
/// @param p_size Size of the buffer in bytes
/// @param p_usage Buffer usage flags
/// @param p_properties Memory type flags
/// @return Vulkan buffer and device memory pointers
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer
	(vk::raii::Device const &p_device, vk::raii::PhysicalDevice const &p_physDevice, vk::DeviceSize p_size, vk::BufferUsageFlags p_usage, vk::MemoryPropertyFlags p_properties);

/// @brief Helper function to start a command to send to queue
/// @param p_device RAII pointer reference to the device
/// @param p_commandPool RAII pointer reference to the Command Pool
/// @return RAII pointer reference to a Command Buffer
vk::raii::CommandBuffer BeginSingleTimeCommands
	(vk::raii::Device const &p_device, vk::raii::CommandPool const &p_commandPool);

/// @brief Helper function to end the command sent to the queue
/// @param p_commandBuffer RAII pointer reference to a Command Buffer
/// @param p_queue RAII pointer reference to the Queue
void EndSingleTimeCommands
	(vk::raii::CommandBuffer &&p_commandBuffer, vk::raii::Queue const &p_queue);

/// @brief Submits a copy command to the queue
/// @param p_device RAII pointer reference to the Vulkan Device
/// @param p_queue RAII pointer reference to the Vulkan Queue
/// @param p_commandPool pointer reference to the Command Pool
/// @param p_srcBuffer Reference to the buffer to copy from
/// @param p_dstBuffer Reference to the buffer to copy to
/// @param p_size Size of the source buffer in bytes
void CopyBuffer
	(vk::raii::Device const &p_device, vk::raii::Queue const &p_queue, vk::raii::CommandPool const &p_commandPool, vk::raii::Buffer &p_srcBuffer, vk::raii::Buffer &p_dstBuffer, vk::DeviceSize p_size);