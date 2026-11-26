#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
	/// @brief Vulkan context abstraction class
	class Context
	{
	public:
		/// @brief Deleted default constructor
		Context() = delete;
		
		Context(nullptr_t p_nullptr)
		{
			m_instance = { p_nullptr };
			m_debugMessenger = { p_nullptr };
			m_surface = { p_nullptr };
			m_physicalDevice = { p_nullptr };
			m_device = { p_nullptr };
			m_queueIndex = { 0 };
			m_queue = { p_nullptr };
			m_requiredDeviceExtension = { p_nullptr };
			m_validationLayers = { p_nullptr };
		}
		
		/// @brief VulkanContext constructor
		/// @param p_window Window to base the surface on
		/// @param p_context Vulkan Context
		Context(SDL_Window *p_window, 
			const vk::raii::Context &p_context);

		Context(const Context &p_other) = delete;
		Context &operator=(const Context &p_other) = delete;
		
		Context(Context &&p_other) noexcept
		{
			std::swap(m_instance, p_other.m_instance);
			std::swap(m_debugMessenger, p_other.m_debugMessenger);
			std::swap(m_surface, p_other.m_surface);
			std::swap(m_physicalDevice, p_other.m_physicalDevice);
			std::swap(m_device, p_other.m_device);
			std::swap(m_queueIndex, p_other.m_queueIndex);
			std::swap(m_queue, p_other.m_queue);

			std::swap(m_requiredDeviceExtension, p_other.m_requiredDeviceExtension);
			std::swap(m_validationLayers, p_other.m_validationLayers);
		}
		
		Context &operator=(Context &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_instance, p_other.m_instance);
			std::swap(m_debugMessenger, p_other.m_debugMessenger);
			std::swap(m_surface, p_other.m_surface);
			std::swap(m_physicalDevice, p_other.m_physicalDevice);
			std::swap(m_device, p_other.m_device);
			std::swap(m_queueIndex, p_other.m_queueIndex);
			std::swap(m_queue, p_other.m_queue);

			std::swap(m_requiredDeviceExtension, p_other.m_requiredDeviceExtension);
			std::swap(m_validationLayers, p_other.m_validationLayers);
			
			return *this;
		}
		
		/// @brief Default destructor
		~Context() = default;
		
		bool operator==(const Context &p_other) const
		{
			return (m_instance == p_other.m_instance								&&
					m_debugMessenger == p_other.m_debugMessenger					&&
					m_surface == p_other.m_surface									&&
					m_physicalDevice == p_other.m_physicalDevice					&&
					m_device == p_other.m_device									&&
					m_queue == p_other.m_queue										&&
					m_requiredDeviceExtension == p_other.m_requiredDeviceExtension	&&
					m_validationLayers == p_other.m_validationLayers);
		}
		
		/// @brief Vulkan Instance Getter
		/// @return Pointer to the vulkan instance
		[[nodiscard]] const vk::raii::Instance &GetInstance() const { return m_instance; }

		/// @brief Vulkan Device getter
		/// @return Pointer to the vulkan device
		[[nodiscard]] const vk::raii::Device &GetDevice() const { return m_device; }

		/// @brief Physical Device getter
		/// @return Pointer to the physical device
		[[nodiscard]] const vk::raii::PhysicalDevice &GetPhysicalDevice() const { return m_physicalDevice; }

		/// @brief Window surface getter
		/// @return Pointer to the Vulkan Window surface
		[[nodiscard]] const vk::raii::SurfaceKHR &GetSurface() const { return m_surface; }

		/// @brief Vulkan queue getter
		/// @return Pointer to the vulkan queue
		[[nodiscard]] const vk::raii::Queue &GetQueue() const { return m_queue; }

		/// @brief Queue index getter
		/// @return uint32 Queue index
		[[nodiscard]] std::uint32_t GetQueueIndex() const { return m_queueIndex; }

		/// @brief Checks if the device supports rectangular smooth lines
		/// @return bool whether device supports rectangular smooth lines (true) or not (false)
		[[nodiscard]] bool SupportsRectangularSmooth();
	private:
		/// @brief Creates the vulkan instance
		void CreateInstance(const vk::raii::Context &p_context);

		/// @brief Picks a physical device (GPU) from the computer to work with
		void PickPhysicalDevice();

		/// @brief Creates the logical version of the hardware to interact with
		void CreateLogicalDevice();

		/// @brief Checks compatibility of the device
		/// @param physicalDevice A physical device candidate
		/// @return bool whether device is suitable (true) or not (false)
		[[nodiscard]] bool IsDeviceSuitable(vk::raii::PhysicalDevice const &physicalDevice);

		/// @brief Prepares the debug messenger
		void SetupDebugMessenger();

		/// @brief Creates the surface linked to an SDL window
		/// @param p_window An SDL window
		void CreateSurface(SDL_Window *p_window);

		vk::raii::Instance						m_instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT		m_debugMessenger = nullptr;
		vk::raii::SurfaceKHR					m_surface = nullptr;
		vk::raii::PhysicalDevice				m_physicalDevice = nullptr;
		vk::raii::Device						m_device = nullptr;
		std::uint32_t							m_queueIndex = static_cast<std::uint32_t>(~0);
		vk::raii::Queue							m_queue = nullptr;

		std::vector<const char *>			m_requiredDeviceExtension = { vk::KHRSwapchainExtensionName, vk::EXTDescriptorIndexingExtensionName };
		std::vector<char const *>			m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
	};
}
