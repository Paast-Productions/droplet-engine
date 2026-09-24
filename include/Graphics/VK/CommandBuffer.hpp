#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
	#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

namespace Droplet::Graphics::VK
{
	class CommandBuffer
	{
	public:
		[[nodiscard]] vk::raii::CommandBuffer& Get();
		void Begin(vk::CommandBufferUsageFlagBits p_flags);
		void End();
		
		void CopyBuffer(vk::Buffer p_src, vk::Buffer p_dst, vk::DeviceSize p_size);
		
	private:
		friend class CommandPool;
		
		CommandBuffer() = delete;
		
		CommandBuffer(
			const vk::raii::Device &p_device,
			const vk::raii::CommandPool &p_pool,
			const vk::CommandBufferLevel p_level);
		
		vk::raii::CommandBuffer m_commandBuffer = nullptr;
	};
}
