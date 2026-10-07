#pragma once

class ShaderModule
{
public:
    ShaderModule() = default;
    ShaderModule(const ShaderModule &p_other) = delete;
    ShaderModule &operator=(const ShaderModule &p_other) = delete;
    ShaderModule(ShaderModule &&p_other) noexcept = delete;
    ShaderModule &operator=(ShaderModule &&p_other) noexcept = delete;

    ~ShaderModule() = default;
    
};
