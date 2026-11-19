#include "Integration/Vulkan/Abstraction/Shader.hpp"

#include <stdexcept>

using namespace Vulkan;

Shader::Shader(const vk::raii::Device& device, const ShaderCreateInfo& info)
    :
    m_stage(info.stage),
    m_entryPoint(info.entrypoint)
{
    if (info.spirv.empty())
    {
        throw std::invalid_argument("SPIRV was empty");
    }
    if (m_entryPoint.empty())
    {
        throw std::invalid_argument("Entry point name cannot be empty");
    }
    
    const vk::ShaderModuleCreateInfo createInfo {
      .codeSize = info.spirv.size() * sizeof(std::uint32_t), // bytes, not words
      .pCode = info.spirv.data()  
    };
    
    m_module = vk::raii::ShaderModule(device, createInfo);
}
