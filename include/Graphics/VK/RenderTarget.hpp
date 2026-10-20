#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Graphics/VK/DepthBuffer.hpp>
#include <Graphics/VK/ImageView.hpp>
namespace Droplet::Graphics
{
	/// @brief A class that represent the endpoints of what the renderer need
	/// for rendering to a texture
    // TODO: Add option to skip depth buffer if not needed.
	class RenderTarget
	{
	public:
		RenderTarget() = delete;
		
		RenderTarget(nullptr_t p_nullptr) :
			m_colourImage(p_nullptr),
			m_colourView(p_nullptr),
			m_depthBuffer(p_nullptr) {}
		
		/// @brief Constructor that create every vulkan resource
		/// @param p_allocator VMA raii Allocator
		/// @param p_device Vulkan device
		/// @param p_physicalDevice Information about the physical computer components
		/// @param p_width Width of the render target texture
		/// @param p_height Height of the render target texture
		RenderTarget(
			const vma::raii::Allocator &p_allocator,
			const vk::raii::Device &p_device,
			const vk::raii::PhysicalDevice &p_physicalDevice,
			std::uint32_t p_width,
			std::uint32_t p_height);

		/// @brief Get the texture
		/// returns A custom Image
		[[nodiscard]] const vk::raii::Image &GetColorImage() const { return m_colourImage.Get(); }
		
		/// @brief Get the depth buffer of the texture
		/// @returns A custom DepthBuffer 
		[[nodiscard]] const vk::raii::Image &GetDepthImage() const { return m_depthBuffer.GetImage(); }
		
		/// @brief Get the descriptor of the texture
		/// @returns A custom ImageView
		[[nodiscard]] const vk::raii::ImageView &GetColorImageView() const { return m_colourView.Get(); }
		
		[[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return m_depthBuffer.GetView(); }
		
		[[nodiscard]] const vk::Format &GetDepthFormat() const { return m_depthBuffer.GetFormat(); }
		
		/// @brief Get the extent of the texture
		/// @returns A Vulkan Extent2D
		[[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }

	private:
		/// @brief Variables that represent a renderTarget
		/// NOTE: The order in which these are created matters
		vk::Extent2D m_extent {};

		VK::Image m_colourImage { nullptr };
		VK::ImageView m_colourView { nullptr };
		VK::DepthBuffer m_depthBuffer { nullptr };

	};
}