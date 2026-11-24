#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <cstdint>
#include <string>

namespace Droplet::Graphics::Vulkan
{
    struct ShaderCreateInfo
    {
        std::vector<std::uint32_t> spirv;
        vk::ShaderStageFlagBits stage{vk::ShaderStageFlagBits::eVertex};
        std::string entrypoint;
    };
    
    class Shader
    {
    public:
        Shader(const vk::raii::Device& device, const ShaderCreateInfo& info);

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) noexcept = default;
        Shader& operator=(Shader&&) noexcept = default;

        [[nodiscard]] vk::ShaderModule Module() const { return *m_module; }
        [[nodiscard]] vk::ShaderStageFlagBits Stage() const { return m_stage; }
        [[nodiscard]] const std::string& EntryPoint() const { return m_entryPoint; }

        /// pName points into this object: the Shader must outlive the pipeline creation call.
        [[nodiscard]] vk::PipelineShaderStageCreateInfo StageInfo() const
        {
            return {
                .stage = m_stage,
                .module = *m_module,
                .pName = m_entryPoint.c_str()
            };
        }

    private:
        vk::raii::ShaderModule m_module{nullptr};
        vk::ShaderStageFlagBits m_stage{};
        std::string m_entryPoint;
    };
}
