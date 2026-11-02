#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

namespace Droplet::Graphics::VK
{
    class PresentImage
    {
    public:
        PresentImage() = delete;
        
        PresentImage(nullptr_t p_nullptr) :
            m_image(p_nullptr) {}
        
        PresentImage(const vk::Image &p_image) :
            m_image(p_image) {}
        
        PresentImage(const PresentImage &p_other) = delete;
        PresentImage &operator=(const PresentImage &p_other) = delete;
        PresentImage(PresentImage &&p_other) noexcept = delete;
        PresentImage &operator=(PresentImage &&p_other) noexcept = delete;

        ~PresentImage() = default;
        
        [[nodiscard]] vk::Image Get() const { return m_image; }
    
    private:
        vk::Image m_image;
    };
}
