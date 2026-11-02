#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <Graphics/VK/ImageView.hpp>
#include <Graphics/VK/DepthBuffer.hpp>

namespace Droplet::Graphics::VK
{
    class PresentTarget
    {
    public:
        PresentTarget() = delete;
        
        PresentTarget(nullptr_t p_nullptr) :
            m_imageView(p_nullptr),
            m_depthBuffer(p_nullptr) {}
        
        /// @brief Constructs a present target
        /// @param p_allocator VMA allocator
        /// @param p_device Vulkan device
        /// @param p_physicalDevice Vulkan physical device
        /// @param p_presentImage Swapchain present image
        /// @param p_surfaceFormat Swapchain present image format
        /// @param p_width Width of present image in pixels
        /// @param p_height Height of present image in pixels
        PresentTarget(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice, vk::Image p_presentImage, vk::Format p_surfaceFormat, std::uint32_t p_width, std::uint32_t p_height);
        
        PresentTarget(const PresentTarget &p_other) = delete;
        PresentTarget &operator=(const PresentTarget &p_other) = delete;
        
        PresentTarget(PresentTarget &&p_other) noexcept
        {
            std::swap(m_extent, p_other.m_extent);    
            std::swap(m_imageView, p_other.m_imageView);    
            std::swap(m_depthBuffer, p_other.m_depthBuffer);    
        }
        
        PresentTarget &operator=(PresentTarget &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_extent, p_other.m_extent);    
            std::swap(m_imageView, p_other.m_imageView);    
            std::swap(m_depthBuffer, p_other.m_depthBuffer);    
            
            return *this;
        }
        
        ~PresentTarget() = default;
        
        bool operator==(const PresentTarget &p_other) const
        {
            return (m_extent == p_other.m_extent            &&
                    m_imageView == p_other.m_imageView      &&
                    m_depthBuffer == p_other.m_depthBuffer);
        }
        
        /// @brief Getter function for the depth image
        /// @returns Depth image
        [[nodiscard]] const vk::raii::Image &GetDepthImage() const { return m_depthBuffer.GetImage(); }
        
        /// @brief Getter function for the present image view
        /// @returns Present image view
        [[nodiscard]] const vk::raii::ImageView &GetPresentImageView() const { return m_imageView.Get(); }
        
        /// @brief Getter function for the depth image view
        /// @returns Depth image view
        [[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return m_depthBuffer.GetView(); }
        
        /// @brief Getter function for the depth image format
        /// @returns Depth image format
        [[nodiscard]] vk::Format GetDepthFormat() const { return m_depthBuffer.GetFormat(); }
        
        /// @brief Getter function for the image extent
        /// @returns Image extent
        [[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }
        
    
    private:
        vk::Extent2D m_extent {};
        
        ImageView m_imageView { nullptr };
        DepthBuffer m_depthBuffer { nullptr };
    };
}
