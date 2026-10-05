#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
    /// @brief Vulkan Image View wrapper class
    class ImageView
    {
    public:
        ImageView() = delete;
    
        /// @brief Image View Nullptr Constructor
        ImageView(nullptr_t p_nullptr)
        {
            m_imageView = { p_nullptr };   
        }
        
        // TODO: <REFACTOR>  
        // EXPLANATION: Should be one singular constructor that can take in all combinations of flags.
        // If you would like a specific combination, create a factory class
        
        /// @brief 
        ImageView(const vk::raii::Device &p_device, 
            const vma::raii::Image &p_image);
        
        ImageView(const vk::raii::Device &p_device, 
            const vma::raii::Image &p_image, 
            vk::Format p_format,
            vk::ImageAspectFlagBits p_aspectFlagBits);
        
        // TODO: </REFACTOR>
        
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
        
        /// @brief Getter-function for a Vulkan Image View
        /// @returns Vulkan Image View
        [[nodiscard]] const vk::raii::ImageView &Get() const;
        
    private:
        vk::raii::ImageView m_imageView { nullptr };
    
    };

    inline const vk::raii::ImageView &ImageView::Get() const
    {
        return m_imageView;
    }
}
