#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
    class ImageView
    {
    public:
        ImageView() = delete;
    
        ImageView(nullptr_t p_nullptr)
        {
            m_imageView = { p_nullptr };   
        }
        
        ImageView(const vk::raii::Device &p_device, 
            const vma::raii::Image &p_image);
        
        ImageView(const vk::raii::Device &p_device, 
            const vma::raii::Image &p_image, 
            vk::Format p_format,
            vk::ImageAspectFlagBits p_aspectFlagBits);
        
        ImageView(const ImageView &p_other) = delete;
        ImageView &operator=(const ImageView &p_other) = delete;
        
        ImageView(ImageView &&p_other) noexcept
        {
            m_imageView = std::move(p_other.m_imageView);
        }
        
        ImageView &operator=(ImageView &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            m_imageView = std::move(p_other.m_imageView);
            
            return *this;
        }
    
        ~ImageView() = default;
    
        bool operator==(const ImageView &p_other) const
        {
            return m_imageView == p_other.m_imageView;
        }
        
        const vk::raii::ImageView &Get();
        
    private:
        vk::raii::ImageView m_imageView { nullptr };
    
    };

    inline const vk::raii::ImageView &ImageView::Get()
    {
        return m_imageView;
    }
}
