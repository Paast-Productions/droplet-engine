#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include "CommandPool.hpp"

/// @brief Helper function to find memory properties of the hardware
/// @param p_physDevice RAII pointer reference to hardware device
/// @param p_typeFilter Type bits of the hardwares memory requirements
/// @param p_properties Memory property flags
/// @returns memory type index
uint32_t FindMemoryType(
	const vk::raii::PhysicalDevice& p_physDevice, 
	uint32_t p_typeFilter, 
	vk::MemoryPropertyFlags p_properties);

/// @brief Helper function to allocate and create a buffer
/// @param p_device RAII pointer reference to the device
/// @param p_physDevice RAII pointer reference to hardware device
/// @param p_size Size of the buffer in bytes
/// @param p_usage Buffer usage flags
/// @param p_properties Memory type flags
/// @returns Vulkan buffer and device memory pointers
std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> CreateBuffer(
	const vk::raii::Device &p_device,
	const vk::raii::PhysicalDevice &p_physDevice, 
	vk::DeviceSize p_size, 
	vk::BufferUsageFlags p_usage, 
	vk::MemoryPropertyFlags p_properties);

/// @brief Submits a copy command to the queue
/// @param p_queue RAII pointer reference to the Vulkan Queue
/// @param p_commandPool pointer reference to a CommandPool instance
/// @param p_srcBuffer Reference to the buffer to copy from
/// @param p_dstBuffer Reference to the buffer to copy to
/// @param p_size Size of the source buffer in bytes
void CopyBuffer(
	const vk::raii::Device &p_device, 
	const vk::raii::Queue &p_queue, 
	Droplet::Graphics::VK::CommandPool &p_commandPool,
	const vk::raii::Buffer &p_srcBuffer, 
	const vk::raii::Buffer &p_dstBuffer, 
	vk::DeviceSize p_size);

/// @brief Translates image data from one layout to another
/// @param p_commandBuffer RAII pointer to a command buffer
/// @param p_image RAII pointer to the image
/// @param p_oldLayout Specified layout to translate from
/// @param p_newLayout Specified layout to translate to
void TransitionImageLayout(
	vk::raii::CommandBuffer &p_commandBuffer,
	const vk::raii::Image &p_image,
	vk::ImageLayout p_oldLayout,
	vk::ImageLayout p_newLayout);
