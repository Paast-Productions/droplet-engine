#pragma once

#include "resource/types/ShaderResource.hpp"

#include <memory>
#include <filesystem>
#include <json/json.hpp>

namespace Droplet::IO::SlangFormat
{
    /// @brief Compiles a shader file (.slang) into a shader blob and loads it into RAM.
    /// @param p_shaderPath The path to the shader file.
    /// @param p_loadSettings The shader specific import settings.
    /// @return A shader resource.
    /// @throw std::runtime_error If the shader compilation failed.
    [[nodiscard]] std::unique_ptr<ShaderResource> CompileAndLoad(const std::filesystem::path &p_shaderPath, const nlohmann::json &p_loadSettings);

    /// @brief Lists the resources in a shader file. This function is just a formality as shader files only contain
    /// one shader.
    /// @param p_shaderPath The path to the shader file.
    /// @return A list of all resources found in the shader file.
    [[nodiscard]] std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::filesystem::path &p_shaderPath);
}
