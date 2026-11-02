#include "ShaderModule.hpp"

Droplet::Graphics::VK::ShaderModule::ShaderModule(const vk::raii::Device &p_device, const Slang::ComPtr<slang::IBlob> &p_shaderBlob)
{
    vk::ShaderModuleCreateInfo createInfo
    {
        .codeSize = p_shaderBlob->getBufferSize(), 
        .pCode = static_cast<const std::uint32_t*>(p_shaderBlob->getBufferPointer())
    };
    
    m_shaderModule = vk::raii::ShaderModule { p_device, createInfo };
}
