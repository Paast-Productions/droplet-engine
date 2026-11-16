#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
	/// @brief CommandBuffer class to abstract creation and usage of Vulkan CommandBuffers.
	class CommandBuffer
	{
	public:
		CommandBuffer() = delete;
		
		/// @brief Initialize a CommandBuffer instance.
		/// @param p_device The device which owns the command pool.
		/// @param p_pool The command pool to allocate the buffer from.
		/// @param p_level The command buffer level.
		CommandBuffer(const vk::raii::Device &p_device,
			const vk::raii::CommandPool &p_pool,
			vk::CommandBufferLevel p_level);
		
		/// @brief Get the Vulkan RAII CommandBuffer associated with the instance. 
		[[nodiscard]] const vk::raii::CommandBuffer &Get() const { return m_commandBuffer; }
		
		/// @brief Set the command buffer to start recording commands.
		/// @param p_flags Specify the usage behavior of the command buffer.
		void Begin(vk::CommandBufferUsageFlagBits p_flags);
		
		/// @brief Complete the recording of the command buffer.
		void End();

	private:
		vk::raii::CommandBuffer m_commandBuffer { nullptr }; 
		
	};
}
