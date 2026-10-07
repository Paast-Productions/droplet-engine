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

	class RenderTarget
	{
	public:
		RenderTarget(
			const vma::raii::Allocator &p_allocator,
			const vk::raii::Device &p_device,
			const vk::raii::PhysicalDevice &p_physicalDevice,
			uint32_t p_width,
			uint32_t p_height);

		VK::Image &GetColorImage();
		VK::ImageView &GetColorImageView();
		VK::DepthBuffer &GetDepthBuffer();
		vk::Extent2D GetExtent() const;

	private:
		vk::Extent2D m_extent;

		VK::Image m_colorImage;
		VK::ImageView m_colorView;
		VK::DepthBuffer m_depthBuffer;

	};
}