#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 

#include <vector>

#include <SDL3/SDL.h>

#include <Graphics/VK/IntermediaryStorage.hpp>
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
			std::swap(m_targetIndices, p_other.m_targetIndices);
			std::swap(m_surfaceFormat, p_other.m_surfaceFormat);
		}
		
		Swapchain &operator=(Swapchain &&p_other) noexcept
		{
			if (*this == p_other)
			{
				return *this;
			}
			
			std::swap(m_swapchain, p_other.m_swapchain);
			std::swap(m_targetIndices, p_other.m_targetIndices);
			std::swap(m_surfaceFormat, p_other.m_surfaceFormat);
			
			return *this;
		}
		
		/// @brief Swapchain Constructor
		/// @param p_device Reference to the device
		/// @param p_physicalDevice Reference to the hardware device
		/// @param p_window Pointer to the window
		/// @param p_swapSurface Reference to the swapchain surface
		Swapchain(const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, SDL_Window *p_window, const vk::raii::SurfaceKHR &p_swapSurface);

		~Swapchain() = default;
		
		[[nodiscard]] bool operator==(const Swapchain &p_other) const
		{
			return (m_swapchain == p_other.m_swapchain				&&
					m_targetIndices == p_other.m_targetIndices		&&
					m_surfaceFormat == p_other.m_surfaceFormat);
		}
		
		/// @brief Acquires the image of the next available presentation image
		/// @param p_device Vulkan device
		/// @param p_semaphore Vulkan semaphore
		/// @param p_fence Vulkan fence
		/// @param p_timeout Time in nanoseconds before swapchain stops waiting for next available image
		/// @returns Index of the next available image
		[[nodiscard]] vk::ResultValue<std::uint32_t> AcquireNextImageIndex(const vk::raii::Device &p_device, const vk::raii::Semaphore &p_semaphore, const vk::raii::Fence &p_fence, std::uint64_t p_timeout);
		
		/// @brief Getter function for the current image index
		/// @returns Current image index
		[[nodiscard]] std::uint32_t GetCurrentImageIndex() const { return m_currentIndex; }
		
		/// @brief Getter function for the total amount of swapchain images
		/// @returns Number of swapchain images
		[[nodiscard]] std::uint32_t GetNumImages() const { return m_targetIndices.size(); }
		
		/// @brief Swapchain getter
		/// @return Reference to the swapchain
		[[nodiscard]] const vk::raii::SwapchainKHR &Get() const { return m_swapchain; }
		
		/// @brief Getter function for the current present image
		/// @returns Present image
		[[nodiscard]] vk::Image GetPresentImage() const { return m_swapchain.getImages().at(m_currentIndex); }
		
		/// @brief Getter function for the current depth image
		/// @returns Depth image
		[[nodiscard]] const vk::raii::Image &GetDepthImage() const { return IntermediaryStorage::GetInstance().GetPresentTargetAt(m_targetIndices.at(m_currentIndex)).GetDepthImage(); }
		
		/// @brief Getter function for the current present image view
		/// @returns Present image view
		[[nodiscard]] const vk::raii::ImageView &GetPresentImageView() const { return IntermediaryStorage::GetInstance().GetPresentTargetAt(m_targetIndices.at(m_currentIndex)).GetPresentImageView(); }
		
		/// @brief Getter function for the current depth image view
		/// @returns Depth image view
		[[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return IntermediaryStorage::GetInstance().GetPresentTargetAt(m_targetIndices.at(m_currentIndex)).GetDepthImageView(); }
		
		/// @brief Getter function for the current depth image format
		/// @returns Depth image format
		[[nodiscard]] vk::Format GetDepthFormat() const { return IntermediaryStorage::GetInstance().GetPresentTargetAt(m_targetIndices.at(m_currentIndex)).GetDepthFormat(); }
		
		/// @brief Getter function for the current present image extent
		/// @returns Image extent
		[[nodiscard]] vk::Extent2D GetExtent() const { return IntermediaryStorage::GetInstance().GetPresentTargetAt(m_targetIndices.at(m_currentIndex)).GetExtent(); } 
		
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
		std::vector<std::uint32_t>		m_targetIndices {}; 
		vk::SurfaceFormatKHR			m_surfaceFormat {};
		
		std::uint32_t m_currentIndex { 0 };
	};
}

