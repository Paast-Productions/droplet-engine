#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <SDL3/SDL.h>

#include <vector>

namespace Droplet::Graphics::VK
{
	/// @brief Swapchain abstraction class
	class Swapchain
	{
	public:
		Swapchain() = delete;
		
		Swapchain(nullptr_t p_nullptr)
		{
			m_swapchain = { p_nullptr };
			m_swapchainImages = {};
			m_swapchainSurfaceFormat = vk::SurfaceFormatKHR{};
			m_swapchainExtent = vk::Extent2D{};
		}
		
		Swapchain(const Swapchain &p_other) = delete;
		Swapchain &operator=(const Swapchain &p_other) = delete;

		Swapchain(Swapchain &&p_other) noexcept
		{
			std::swap(m_swapchain, p_other.m_swapchain);
			std::swap(m_swapchainImages, p_other.m_swapchainImages);
			std::swap(m_swapchainImageViews, p_other.m_swapchainImageViews);
			std::swap(m_swapchainSurfaceFormat, p_other.m_swapchainSurfaceFormat);
			std::swap(m_swapchainExtent, p_other.m_swapchainExtent);
		}
		
		Swapchain &operator=(Swapchain &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_swapchain, p_other.m_swapchain);
			std::swap(m_swapchainImages, p_other.m_swapchainImages);
			std::swap(m_swapchainImageViews, p_other.m_swapchainImageViews);
			std::swap(m_swapchainSurfaceFormat, p_other.m_swapchainSurfaceFormat);
			std::swap(m_swapchainExtent, p_other.m_swapchainExtent);
			
			return *this;
		}
		
		/// @brief Swapchain Constructor
		/// @param p_device Pointer to the device
		/// @param p_physicalDevice Pointer to the hardware device
		/// @param p_window Reference to the window
		/// @param p_swapSurface Pointer to the swapchain surface
		Swapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface);

		~Swapchain() = default;
		
		[[nodiscard]] bool operator==(const Swapchain &p_other) const
		{
			return (m_swapchain == p_other.m_swapchain								&&
					m_swapchainImages == p_other.m_swapchainImages					&&
					m_swapchainImageViews == p_other.m_swapchainImageViews			&&
					m_swapchainSurfaceFormat == p_other.m_swapchainSurfaceFormat	&&
					m_swapchainExtent == p_other.m_swapchainExtent);
		}
		
		/// @brief Clears the swapchain images and sets the pointer to nullptr
		void Cleanup(const vk::raii::Device &p_device);

		/// Creates a new swapchain and images just like the constructor
		void Recreate(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_surface);

		/// @brief Swapchain extent getter
		/// @return The swapchain extent
		[[nodiscard]] vk::Extent2D GetExtent() const;

		/// @brief Surface format getter
		/// @return Reference to the swapchain surface format
		[[nodiscard]] const vk::SurfaceFormatKHR &GetSurfaceFormat() const;

		/// @brief Swapchain imageview getter
		/// @return Pointer to the swapchain image views
		[[nodiscard]] const std::vector<vk::raii::ImageView> &GetImageViews() const;

		/// @brief Swapchain iamge getter
		/// @return Pointer to the swapchain images
		[[nodiscard]] const std::vector<vk::Image> &GetImages() const;

		/// @brief Swapchain getter
		/// @return Pointer to the swapchain
		[[nodiscard]] const vk::raii::SwapchainKHR &Get() const;

	private:

		/// @brief Class helper function for creating/recreating the swapchain
		/// @param p_device Pointer to the vulkan device
		/// @param p_physicalDevice Pointer to the hardware device
		/// @param p_window Reference to the SDL window
		/// @param p_swapSurface Pointer to the swapchain surface
		void CreateSwapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface);

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
		vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities, SDL_Window *p_window);

		/// @brief Chooses a swapchain present mode from available options
		/// @param p_availablePresentModes Available present mode options
		/// @return The chosen present mode
		vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &p_availablePresentModes);

		/// @brief Creates swapchain image views
		/// @param p_device Pointer to the vulkan device
		void CreateImageViews(const vk::raii::Device &p_device);

		vk::raii::SwapchainKHR				m_swapchain = nullptr;
		std::vector<vk::Image>				m_swapchainImages;
		std::vector<vk::raii::ImageView>    m_swapchainImageViews;
		vk::SurfaceFormatKHR				m_swapchainSurfaceFormat;
		vk::Extent2D						m_swapchainExtent;
	};

	[[nodiscard]] inline vk::Extent2D Swapchain::GetExtent() const
	{
		return m_swapchainExtent;
	}

	[[nodiscard]] inline const vk::SurfaceFormatKHR &Swapchain::GetSurfaceFormat() const
	{
		return m_swapchainSurfaceFormat;
	}

	[[nodiscard]] inline const std::vector<vk::raii::ImageView> &Swapchain::GetImageViews() const
	{
		return m_swapchainImageViews;
	}

	[[nodiscard]] inline const std::vector<vk::Image> &Swapchain::GetImages() const
	{
		return m_swapchainImages;
	}

	[[nodiscard]] inline const vk::raii::SwapchainKHR &Swapchain::Get() const
	{
		return m_swapchain;
	}
}

