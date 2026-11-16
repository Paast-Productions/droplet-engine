#include "RenderTarget.hpp"

using namespace Droplet::Graphics;

RenderTarget::RenderTarget(
	DynamicResourceHandle<Vulkan::Image> p_colourImage,
	DynamicResourceHandle<Vulkan::ImageView> p_colourImageView,
	DynamicResourceHandle<Vulkan::Image> p_depthImage,
	DynamicResourceHandle<Vulkan::ImageView> p_depthImageView) : 
		m_colourImageHandle(p_colourImage),
		m_depthImageHandle(p_depthImage),
		m_colourImageViewHandle(p_colourImageView),
		m_depthImageViewHandle(p_depthImageView) {}