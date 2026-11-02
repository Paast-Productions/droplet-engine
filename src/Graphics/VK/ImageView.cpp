#include "ImageView.hpp"

using namespace Droplet::Graphics::VK;

ImageView::ImageView(const vk::raii::Device &p_device, const vk::Image p_image)
{
    const vk::ImageViewCreateInfo viewInfo
    {
        .image = p_image,
        .viewType = vk::ImageViewType::e2D,
        .format = vk::Format::eR8G8B8A8Unorm,
        .subresourceRange = 
        {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        } 
    };
	
    m_imageView = vk::raii::ImageView { p_device, viewInfo };
}

ImageView::ImageView(const vk::raii::Device &p_device, const vk::Image p_image, const vk::Format p_format, const vk::ImageAspectFlagBits p_aspectFlagBits)
{
    const vk::ImageViewCreateInfo viewInfo 
    {
        .image = p_image,
        .viewType = vk::ImageViewType::e2D,
        .format = p_format,
        .subresourceRange =
        {
            .aspectMask = p_aspectFlagBits,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };
    
    m_imageView = vk::raii::ImageView { p_device, viewInfo };
}
