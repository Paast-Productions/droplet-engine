#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <SDL3/SDL_vulkan.h>

namespace Droplet::Graphics::VK
{
	class VulkanContext
	{
	public:
		VulkanContext() = delete;
		~VulkanContext() = default;

		VulkanContext(SDL_Window &p_window);

		[[nodiscard]] const vk::raii::Instance *GetInstance();
		[[nodiscard]] const vk::raii::Device *GetDevice();
		[[nodiscard]] const vk::raii::PhysicalDevice *GetPhysicalDevice();
		[[nodiscard]] const vk::raii::SurfaceKHR *GetSurface();
		[[nodiscard]] const vk::raii::Queue *GetQueue();
		[[nodiscard]] const std::uint32_t GetQueueIndex();
	private:
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		bool IsDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);
		void SetupDebugMessenger();
		void CreateSurface(SDL_Window &p_window);

		vk::raii::Context						m_context;
		vk::raii::Instance						m_instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT		m_debugMessenger = nullptr;
		vk::raii::SurfaceKHR					m_surface = nullptr;
		vk::raii::PhysicalDevice				m_physicalDevice = nullptr;
		vk::raii::Device						m_device = nullptr;
		std::uint32_t							m_queueIndex = static_cast<std::uint32_t>(~0);
		vk::raii::Queue							m_queue = nullptr;

		std::vector<const char *>			 m_requiredDeviceExtension = { vk::KHRSwapchainExtensionName };
	};

	inline const vk::raii::Instance *VulkanContext::GetInstance()
	{
		return &m_instance;
	}

	inline const vk::raii::Device *VulkanContext::GetDevice()
	{
		return &m_device;
	}

	inline const vk::raii::PhysicalDevice *VulkanContext::GetPhysicalDevice()
	{
		return &m_physicalDevice;
	}

	inline const vk::raii::SurfaceKHR *VulkanContext::GetSurface()
	{
		return &m_surface;
	}

	inline const vk::raii::Queue *VulkanContext::GetQueue()
	{
		return &m_queue;
	}

	inline const std::uint32_t VulkanContext::GetQueueIndex()
	{
		return m_queueIndex;
	}
}
