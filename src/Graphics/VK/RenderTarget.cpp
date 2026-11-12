#include "RenderTarget.hpp"

using namespace Droplet::Graphics;

RenderTarget::RenderTarget(
	DynamicResourceHandle<VK::Image> p_colourImage,
	DynamicResourceHandle<VK::ImageView> p_colourImageView,
	DynamicResourceHandle<VK::Image> p_depthImage,
	DynamicResourceHandle<VK::ImageView> p_depthImageView) : 
		m_colourImageHandle(p_colourImage),
		m_depthImageHandle(p_depthImage),
		m_colourImageViewHandle(p_colourImageView),
		m_depthImageViewHandle(p_depthImageView) {}