#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
	#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
	/// @brief CommandBuffer class to abstract creation and usage of Vulkan CommandBuffers.
	class CommandBuffer
	{
	public:
		/// @brief Get the Vulkan RAII CommandBuffer associated with the instance. 
		[[nodiscard]] vk::raii::CommandBuffer& Get();
		
		/// @brief Set the command buffer to start recording commands.
		/// @param p_flags Specify the usage behavior of the command buffer.
		void Begin(vk::CommandBufferUsageFlagBits p_flags);
		
		/// @brief Complete the recording of the command buffer.
		void End();

		/// @brief Issue command to copy data between two buffers.
		/// @param p_src The source buffer.
		/// @param p_dst The destination buffer.
		/// @param p_size The number of bytes to copy.
		/// @note Buffers are *not* the same as command buffers. The command buffer is only a middle-hand to copy data.
		void CopyBuffer(vk::Buffer p_src, vk::Buffer p_dst, vk::DeviceSize p_size);

		/// @brief Sends dependency information to the buffer
		/// @param p_dependencyInfo The dependency information
		void PipelineBarrier(vk::DependencyInfo p_dependencyInfo);
		
	private:
		friend class CommandPool;
		
		CommandBuffer() = delete;

		/// @brief Initialize a CommandBuffer instance.
		/// @param p_device The device which owns the command pool.
		/// @param p_pool The command pool to allocate the buffer from.
		/// @param p_level The command buffer level.
		CommandBuffer(
			const vk::raii::Device &p_device,
			const vk::raii::CommandPool &p_pool,
			const vk::CommandBufferLevel p_level);
		
		vk::raii::CommandBuffer m_commandBuffer = nullptr;
	};
}
