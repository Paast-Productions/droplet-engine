#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>

#include <SDL3/SDL.h>

#include <vector>

namespace Droplet::Graphics::VK
{
	/// @brief Swapchain abstraction class
	class Swapchain
	{
	public:
		/// @brief Deleted default constructor
		Swapchain() = delete;

		/// @brief Deafult destructor
		~Swapchain() = default;

		/// @brief Swapchain Constructor
		/// @param p_device Pointer to the device
		/// @param p_physicalDevice Pointer to the hardware device
		/// @param p_window Reference to the window
		/// @param p_swapSurface Pointer to the swapchain surface
		Swapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window &p_window, const vk::raii::SurfaceKHR &p_swapSurface);

		/// @brief Clears the swapchain images and sets the pointer to nullptr
		void Cleanup();

		/// Creates a new swapchain and images just like the constructor
		void Recreate(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window &p_window, const vk::raii::SurfaceKHR &p_surface);

		/// @brief Swapchain extent getter
		/// @return The swapchain extent
		const vk::Extent2D GetExtent();

		/// @brief Surface format getter
		/// @return Reference to the swapchain surface format
		const vk::SurfaceFormatKHR &GetSurfaceFormat();

		/// @brief Swapchain imageview getter
		/// @return Pointer to the swapchain image views
		const std::vector<vk::raii::ImageView> *GetImageViews();

		/// @brief Swapchain iamge getter
		/// @return Pointer to the swapchain images
		const std::vector<vk::Image> *GetImages();

		/// @brief Swapchain getter
		/// @return Pointer to the swapchain
		const vk::raii::SwapchainKHR *GetSwapchain();
	private:

		/// @brief Class helper function for creating/recreating the swapchain
		/// @param p_device Pointer to the vulkan device
		/// @param p_physicalDevice Pointer to the hardware device
		/// @param p_window Reference to the SDL window
		/// @param p_swapSurface Pointer to the swapchain surface
		void CreateSwapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window &p_window, const vk::raii::SurfaceKHR &p_swapSurface);

		/// @brief Chooses the swapchain surface format amongst the available options
		/// @param p_availableFormats Available swapchain surface format options
		/// @return The chosen format
		vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_availableFormats);

		/// @brief Chooses the minimum amount of swapchain images dependent on surface capabilities
		/// @param p_surfaceCapabilities Capabilities of the surface
		/// @return Minimum amount of swapchain images
		std::uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surfaceCapabilities);

		/// @brief Choose swapchain extent depending on window size
		/// @param capabilities The capabilities of the swapchain surface
		/// @param p_window Reference to the SDL window
		/// @return The new swapchain extent
		vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities, SDL_Window &p_window);

		/// @brief Chooses a swapchain present mode from available options
		/// @param p_availablePresentModes Available present mode options
		/// @return The chosen present mode
		vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &p_availablePresentModes);

		/// @brief Creates swapchain image views
		/// @param p_device Pointer to the vulkan device
		void CreateImageViews(const vk::raii::Device &p_device);

		vk::raii::SwapchainKHR				 m_swapchain = nullptr;
		std::vector<vk::Image>				 m_swapchainImages;
		std::vector<vk::raii::ImageView>     m_swapchainImageViews;
		vk::SurfaceFormatKHR				 m_swapchainSurfaceFormat;
		vk::Extent2D						 m_swapchainExtent;
	};

	[[nodiscard]] inline const vk::Extent2D Swapchain::GetExtent()
	{
		return m_swapchainExtent;
	}

	[[nodiscard]] inline const vk::SurfaceFormatKHR &Swapchain::GetSurfaceFormat()
	{
		return m_swapchainSurfaceFormat;
	}

	[[nodiscard]] inline const std::vector<vk::raii::ImageView> *Swapchain::GetImageViews()
	{
		return &m_swapchainImageViews;
	}

	[[nodiscard]] inline const std::vector<vk::Image> *Swapchain::GetImages()
	{
		return &m_swapchainImages;
	}

	[[nodiscard]] inline const vk::raii::SwapchainKHR *Swapchain::GetSwapchain()
	{
		return &m_swapchain;
	}
}

