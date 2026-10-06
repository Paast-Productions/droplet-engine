#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <SDL3/SDL_vulkan.h>

namespace Droplet::Graphics::VK
{
	/// @brief Vulkan context abstraction class
	class VulkanContext
	{
	public:
		/// @brief Deleted default constructor
		VulkanContext() = delete;

		/// @brief Default destructor
		~VulkanContext() = default;

		/// @brief VulkanContext constructor
		/// @param p_window Window to base the surface on
		VulkanContext(SDL_Window &p_window);

		/// @brief Vulkan Instance Getter
		/// @return Pointer to the vulkan instance
		[[nodiscard]] const vk::raii::Instance *GetInstance();

		/// @brief Vulkan Device getter
		/// @return Pointer to the vulkan device
		[[nodiscard]] const vk::raii::Device *GetDevice();

		/// @brief Physical Device getter
		/// @return Pointer to the physical device
		[[nodiscard]] const vk::raii::PhysicalDevice *GetPhysicalDevice();

		/// @brief Window surface getter
		/// @return Pointer to the Vulkan Window surface
		[[nodiscard]] const vk::raii::SurfaceKHR *GetSurface();

		/// @brief Vulkan queue getter
		/// @return Pointer to the vulkan queue
		[[nodiscard]] const vk::raii::Queue *GetQueue();

		/// @brief Queue index getter
		/// @return uint32 Queue index
		[[nodiscard]] const std::uint32_t GetQueueIndex();
	private:
		/// @brief Creates the vulkan instance
		void CreateInstance();

		/// @brief Picks a physical device (GPU) from the computer to work with
		void PickPhysicalDevice();

		/// @brief Creates the logical version of the hardware to interact with
		void CreateLogicalDevice();

		/// @brief Checks compatibility of the device
		/// @param physicalDevice A physical device candidate
		/// @return bool whether device is suitable (true) or not (false)
		bool IsDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);

		/// @brief Prepares the debug messenger
		void SetupDebugMessenger();

		/// @brief Creates the surface linked to an SDL window
		/// @param p_window An SDL window
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
