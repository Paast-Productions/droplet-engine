#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace Droplet::Graphics::Vulkan
{
    class PresentImage
    {
    public:
        PresentImage() = delete;
        
        PresentImage(nullptr_t p_nullptr) :
            m_image(p_nullptr) {}
        
        /// @brief PresentImage constructor
        /// @param p_image Vulkan Image
        PresentImage(const vk::Image &p_image) :
            m_image(p_image) {}
        
        PresentImage(const PresentImage &p_other) = delete;
        PresentImage &operator=(const PresentImage &p_other) = delete;
        PresentImage(PresentImage &&p_other) noexcept = delete;
        PresentImage &operator=(PresentImage &&p_other) noexcept = delete;

        ~PresentImage() = default;
        
        /// @brief Getter-function for swapchain image
        [[nodiscard]] vk::Image Get() const { return m_image; }
    
    private:
        vk::Image m_image;
    };
}
