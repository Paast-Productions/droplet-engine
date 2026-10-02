#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
	#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vector>
#include <functional>

#include "CommandBuffer.hpp"

namespace Droplet::Graphics::VK
{
	/// @brief Information which can be used to find a specific command buffer in the pool.
	struct CommandBufferId
	{
		std::size_t Index;
	};
	
	/// @brief CommandPool class to abstract creation and usage of Vulkan CommandPools.
	class CommandPool
	{
	public:
		/// @brief Deleted constructor.
		CommandPool() = delete;
		
		/// @brief Deleted constructor.
		CommandPool(const CommandPool&) = delete;
		
		/// @brief Deleted constructor.
		CommandPool& operator=(const CommandPool&) = delete;
		
		/// @brief CommandPool constructor.
		/// @param p_device RAII pointer to the Vulkan Device.
		/// @param p_queueFamilyIndex Index to a queue family which the command buffers should be submitted to.
		/// @param p_flags Flags to create the command pool with.
		CommandPool(const vk::raii::Device &p_device, const uint32_t p_queueFamilyIndex,
			const vk::CommandPoolCreateFlags p_flags);
		
		/// @brief Create a CommandBuffer in the pool.
		/// @param p_level Level of the command buffer to allocate.
		/// @returns CommandBufferId struct.
		[[nodiscard]] CommandBufferId Allocate(const vk::CommandBufferLevel p_level = vk::CommandBufferLevel::ePrimary);
		
		/// @brief Create multiple CommandBuffers in the pool.
		/// @param p_level Level of the command buffers to allocate.
		/// @param p_count Number of buffers to allocate.
		/// @returns Vector with CommandBufferId structs.
		[[nodiscard]] std::vector<CommandBufferId> Allocate(uint32_t p_count, vk::CommandBufferLevel p_level = vk::CommandBufferLevel::ePrimary);
		
		/// @brief Getter-function for a command buffer matching a descriptor.
		/// @param p_id Struct containing command buffer index in command pool.
		/// @returns Reference to command buffer.
		[[nodiscard]] CommandBuffer& GetBuffer(const CommandBufferId p_id);
		
		/// @brief Submit commands to be immediately submitted to the supplied queue.
		/// 
		/// @tparam RecordFunction Callable type which accepts a CommandBuffer reference.
		/// 
		/// @param p_queue The queue to submit the command to.
		/// @param p_recordFunction Function to be invoked while the command buffer is recorded.
		/// 
		/// @pre The queue represented by p_queue must be compatible with the queue family used by this pool.
		/// @pre p_recordFunction must be passed with a CommandBuffer as a parameter.
		/// @note This function blocks until the queue is idle.
		template<typename RecordFunction>
		requires std::invocable<RecordFunction&, CommandBuffer&>
		void ImmediateSubmit(const vk::raii::Queue &p_queue, RecordFunction p_recordFunction) const;
		
	private:
		const vk::raii::Device &m_device;
		vk::raii::CommandPool m_commandPool = nullptr;
		
		std::vector<CommandBuffer> m_commandBuffers;
	};
	
	template<typename RecordFunction>
	requires std::invocable<RecordFunction&, CommandBuffer&>
	void CommandPool::ImmediateSubmit(const vk::raii::Queue &p_queue, RecordFunction p_recordFunction) const
	{
		CommandBuffer commandBuffer{m_device, m_commandPool, vk::CommandBufferLevel::ePrimary};
		commandBuffer.Begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	
		p_recordFunction(commandBuffer);
	
		commandBuffer.End();
	
		vk::SubmitInfo submitInfo {
			.commandBufferCount = 1,
			.pCommandBuffers = &*commandBuffer.Get()
		};
	
		p_queue.submit(submitInfo);
		p_queue.waitIdle(); // TODO: There is a risk that this becomes a bottleneck. Reconsider the suitability further up.
	}

}
