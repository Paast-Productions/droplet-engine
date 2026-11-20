#pragma once

#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>

#include <Graphics/Vulkan/Image.hpp>
#include <Graphics/Vulkan/ImageView.hpp>
#include <Resource/DynamicResourceHandle.hpp>

namespace Droplet::Graphics::Vulkan
{
	class IntermediaryStorage;
}

namespace Droplet::Graphics
{
	/// @brief A class that represent the endpoints of what the renderer need
	/// for rendering to a texture
    // TODO: Add option to skip depth buffer if not needed.
    class RenderTarget
    {
    public:
        RenderTarget() = default;

		RenderTarget(const RenderTarget &p_other) = default;
		RenderTarget &operator=(const RenderTarget &p_other) = default;
		RenderTarget(RenderTarget &&p_other) noexcept = default;
		RenderTarget &operator=(RenderTarget &&p_other) noexcept = default;

        ~RenderTarget() = default;

		//
		// TODO: <REFACTOR>
		// This should be extended to also returning handles.
		// The handles should then return these resources to a friended Intermediary Storage instead as to 
		// not expose Vulkan resources outside the Intermediary Storage and renderer
		//
		
		/// @brief Get the texture
		/// returns A custom Image
		[[nodiscard]] const vk::raii::Image &GetColourImage() const { return m_colourImageHandle->Get(); }
		
		[[nodiscard]] GUID GetColourImageGuid() const { return m_colourImageHandle.GetGUID(); }
		
		[[nodiscard]] vk::Format GetColourFormat() const { return m_colourImageHandle->GetFormat(); }
		
		/// @brief Get the depth buffer of the texture
		/// @returns A custom DepthBuffer 
		[[nodiscard]] const vk::raii::Image &GetDepthImage() const { return m_depthImageHandle->Get(); }
		
		[[nodiscard]] GUID GetDepthImageGuid() const { return m_depthImageHandle.GetGUID(); }
		
		[[nodiscard]] vk::Format GetDepthFormat() const { return m_depthImageHandle->GetFormat(); }

		/// @brief Get the descriptor of the texture
		/// @returns A custom ImageView
		[[nodiscard]] const vk::raii::ImageView &GetColourImageView() const { return m_colourImageViewHandle->Get(); }
		
		[[nodiscard]] GUID GetColourImageViewGuid() const { return m_colourImageViewHandle.GetGUID(); }
		
		[[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return m_depthImageViewHandle->Get(); }
		
		[[nodiscard]] GUID GetDepthImageViewGuid() const { return m_depthImageViewHandle.GetGUID(); }
		
		/// @brief Get the extent of the texture
		/// @returns A Vulkan Extent2D
		[[nodiscard]] vk::Extent2D GetExtent() const { return m_colourImageHandle->GetExtent(); }
		
		//
		// TODO: </REFACTOR> 
		//

    private:
		// Friend to access data
		friend class Vulkan::IntermediaryStorage;
		
		/// @brief Constructor that create every vulkan resource
		/// @param p_allocator VMA raii Allocator
		/// @param p_device Vulkan device
		/// @param p_physicalDevice Information about the physical computer components
		/// @param p_width Width of the render target texture
		/// @param p_height Height of the render target texture
		RenderTarget(
			DynamicResourceHandle<Vulkan::Image> p_colourImage,
			DynamicResourceHandle<Vulkan::ImageView> p_colourImageView,
			DynamicResourceHandle<Vulkan::Image> p_depthImage,
			DynamicResourceHandle<Vulkan::ImageView> p_depthImageView);
		
		
		/// @brief Variables that represent a renderTarget
		/// NOTE: The order in which these are created matters
		
		// Colour Attachment
		DynamicResourceHandle<Vulkan::Image> m_colourImageHandle {};
		DynamicResourceHandle<Vulkan::ImageView> m_colourImageViewHandle {};
		
		// Depth Attachment
		DynamicResourceHandle<Vulkan::Image> m_depthImageHandle {};
		DynamicResourceHandle<Vulkan::ImageView> m_depthImageViewHandle {};
		
    };
}
