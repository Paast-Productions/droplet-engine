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
        
        PresentTarget(const vma::raii::Allocator &p_allocator, const vk::raii::Device &p_device, const vk::raii::PhysicalDevice &p_physicalDevice,  vk::Image p_presentImage, vk::Format p_surfaceFormat, vk::Extent2D p_extent);
        
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
        
        [[nodiscard]] const vk::raii::Image &GetDepthImage() const { return m_depthBuffer.GetImage(); }
        
        [[nodiscard]] const vk::raii::ImageView &GetPresentImageView() const { return m_imageView.Get(); }
        [[nodiscard]] const vk::raii::ImageView &GetDepthImageView() const { return m_depthBuffer.GetView(); }
        
        [[nodiscard]] vk::Format GetDepthFormat() const { return m_depthBuffer.GetFormat(); }
        [[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }
        
    
    private:
        vk::Extent2D m_extent {};
        
        ImageView m_imageView { nullptr };
        DepthBuffer m_depthBuffer { nullptr };
    };
}
