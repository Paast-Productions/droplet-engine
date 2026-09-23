#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vector>

#include "CommandBuffer.hpp"

namespace Droplet::Graphics::VK
{
	// TODO: Improve this doxygen 
	/// @brief Information about a command buffer
	struct CommandBufferId
	{
		std::size_t Index;
	};
	
	class CommandPool
	{
	public:
		/// @brief Deleted constructor
		CommandPool() = delete;
		
		/// @brief CommandPool constructor
		/// @param p_device RAII pointer to the Vulkan Device
		/// @param p_queueFamilyIndex Index to a queue family which the command buffers should be submitted to
		/// @param p_flags Flags to create the command pool with
		CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex,
			const vk::CommandPoolCreateFlags p_flags);
		
		/// @brief Create a CommandBuffer in the pool
		/// @param p_level Level of the command buffer to allocate
		/// @returns CommandBufferId struct
		CommandBufferId Allocate(const vk::CommandBufferLevel p_level = vk::CommandBufferLevel::ePrimary);
		
		/// @brief Getter-function for a command buffer matching a descriptor
		/// @param p_id Struct containing command buffer index in command pool
		/// @returns Reference to command buffer
		CommandBuffer& Get(const CommandBufferId p_id);
		
	private:
		const vk::raii::Device &m_device;
		vk::raii::CommandPool m_commandPool = nullptr;
		
		std::vector<CommandBuffer> m_commandBuffers;
	};

}
