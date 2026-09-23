#pragma once

#include "resource/types/ShaderResource.hpp"
#include "graphics/vk/ShaderCompiler.hpp"

#include <memory>
#include <filesystem>

namespace Droplet
{
    class ShaderLoader
    {
    public:
        ShaderLoader() = default;
        ~ShaderLoader() = default;

        /// @brief Compiles a shader file (.slang) into a shader blob and loads it into RAM.
        /// @param p_type The shader type (should be taken from the meta entry).
        /// @param p_shaderPath The path to the shader file.
        /// @return A shader resource.
        std::unique_ptr<ShaderResource> CompileAndLoad(ShaderResource::ShaderType p_type, const std::filesystem::path &p_shaderPath);

        /// @brief Lists the resources in a shader file. This function is just a formality as shader files only contain
        /// one shader.
        /// @param p_shaderPath The path to the shader file.
        /// @return A list of all resources found in the shader file.
        std::vector<std::pair<ResourceType, std::string>> ListAssetResources(const std::filesystem::path &p_shaderPath);
        
    private:
        Graphics::ShaderCompiler m_compiler;
    };
}
