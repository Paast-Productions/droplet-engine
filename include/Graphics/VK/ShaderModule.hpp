#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS 
#include <vulkan/vulkan_raii.hpp>
#undef VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <slang/slang.h>
#include <slang/slang-com-ptr.h>

namespace Droplet::Graphics::VK
{
    class ShaderModule
    {
    public:
        ShaderModule() = delete;
        
        ShaderModule(nullptr_t p_nullptr) : 
            m_shaderModule { p_nullptr } {}
        
        ShaderModule(const vk::raii::Device &p_device, const Slang::ComPtr<slang::IBlob> &p_shaderBlob);
        
        ShaderModule(const ShaderModule &p_other) = delete;
        ShaderModule &operator=(const ShaderModule &p_other) = delete;
        
        ShaderModule(ShaderModule &&p_other) noexcept
        {
            std::swap(m_shaderModule, p_other.m_shaderModule);
        }
        
        ShaderModule &operator=(ShaderModule &&p_other) noexcept
        {
            if (*this == p_other)
            {
                return *this;
            }
            
            std::swap(m_shaderModule, p_other.m_shaderModule);
            
            return *this;
        }

        ~ShaderModule() = default;
        
        bool operator==(const ShaderModule &p_other) const
        {
            return m_shaderModule == p_other.m_shaderModule;   
        }
        
        [[nodiscard]] const vk::raii::ShaderModule &Get() const { return m_shaderModule; } 
    
    private:
        vk::raii::ShaderModule m_shaderModule { nullptr };
    };
}

