#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <vector>

#include <SDL3/SDL.h>

#include <Graphics/VK/PresentTarget.hpp>

namespace Droplet::Graphics::VK
{
	/// @brief Swapchain abstraction class
	class Swapchain
	{
	public:
		Swapchain() = delete;
		
		/// @brief Swapchain Nullptr Constructor
		Swapchain(nullptr_t p_nullptr)
		{
			m_swapchain = { p_nullptr };
			m_surfaceFormat = vk::SurfaceFormatKHR{};
		}
		
		Swapchain(const Swapchain &p_other) = delete;
		Swapchain &operator=(const Swapchain &p_other) = delete;

		Swapchain(Swapchain &&p_other) noexcept
		{
			std::swap(m_swapchain, p_other.m_swapchain);
			std::swap(m_targets, p_other.m_targets);
			std::swap(m_surfaceFormat, p_other.m_surfaceFormat);
		}
		
		Swapchain &operator=(Swapchain &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_swapchain, p_other.m_swapchain);
			std::swap(m_targets, p_other.m_targets);
			std::swap(m_surfaceFormat, p_other.m_surfaceFormat);
			
			return *this;
		}
		
		/// @brief Swapchain Constructor
		/// @param p_allocator VMA Allocator
		/// @param p_device Reference to the device
		/// @param p_physicalDevice Reference to the hardware device
		/// @param p_window Pointer to the window
		/// @param p_swapSurface Reference to the swapchain surface
		Swapchain(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface);

		~Swapchain() = default;
		
		[[nodiscard]] bool operator==(const Swapchain &p_other) const
		{
			return (m_swapchain == p_other.m_swapchain			&&
					m_targets == p_other.m_targets				&&
					m_surfaceFormat == p_other.m_surfaceFormat);
		}
		
		void AcquireNextImageIndex(const vk::raii::Device &p_device, const vk::raii::Semaphore &p_semaphore, const vk::raii::Fence &p_fence, uint64_t
		                           p_timeout);
		[[nodiscard]] std::uint32_t GetCurrentIndex() const { return m_currentIndex; }
		[[nodiscard]] std::uint32_t GetNumImages() const { return m_targets.size(); }
		
		/// @brief Swapchain getter
		/// @return Reference to the swapchain
		[[nodiscard]] const vk::raii::SwapchainKHR &Get() const { return m_swapchain; }
		
		[[nodiscard]] vk::Image GetPresentImage() const { return m_swapchain.getImages().at(m_currentIndex); }
		[[nodiscard]] const vk::raii::Image &GetDepthImage() const { return m_targets.at(m_currentIndex).GetDepthImage(); }
		
		[[nodiscard]] const vk::raii::ImageView &GetPresentImageView() const { return m_targets.at(m_currentIndex).GetPresentImageView(); }
		[[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return m_targets.at(m_currentIndex).GetDepthImageView(); }
		
		[[nodiscard]] vk::Format GetDepthFormat() const { return m_targets.at(m_currentIndex).GetDepthFormat(); }
		[[nodiscard]] vk::Extent2D GetExtent() const { return m_targets.at(m_currentIndex).GetExtent(); } 
		
		/// @brief Surface format getter
		/// @return Reference to the swapchain surface format
		[[nodiscard]] vk::SurfaceFormatKHR GetSurfaceFormat() const { return m_surfaceFormat; }
		


	private:

		/// @brief Chooses the swapchain surface format amongst the available options
		/// @param p_availableFormats Available swapchain surface format options
		/// @return The chosen format
		[[nodiscard]] vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &p_availableFormats);

		/// @brief Chooses the minimum amount of swapchain images dependent on surface capabilities
		/// @param p_surfaceCapabilities Capabilities of the surface
		/// @return Minimum amount of swapchain images
		[[nodiscard]] std::uint32_t ChooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &p_surfaceCapabilities);

		/// @brief Choose swapchain extent depending on window size
		/// @param p_capabilities The capabilities of the swapchain surface
		/// @param p_window Reference to the SDL window
		/// @return The new swapchain extent
		[[nodiscard]] vk::Extent2D ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const &p_capabilities, SDL_Window *p_window);

		/// @brief Chooses a swapchain present mode from available options
		/// @param p_availablePresentModes Available present mode options
		/// @return The chosen present mode
		[[nodiscard]] vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &p_availablePresentModes);

		vk::raii::SwapchainKHR			m_swapchain { nullptr };
		std::vector<PresentTarget>		m_targets {}; 
		vk::SurfaceFormatKHR			m_surfaceFormat {};
		
		std::uint32_t m_currentIndex { 0 };
	};
}

