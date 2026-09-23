#pragma once
#include "resource/loaders/ShaderLoader.hpp"

namespace Droplet
{
    std::unique_ptr<ShaderResource> ShaderLoader::CompileAndLoad(ShaderResource::ShaderType p_type, const std::filesystem::path &p_shaderPath)
    {
        try
        {
            return std::make_unique<ShaderResource>(p_type, m_compiler.CompileShader(p_shaderPath));
        }
        catch (std::exception&)
        {
            // TODO: Log the error
            return nullptr;
        }
    }

    std::vector<std::pair<ResourceType, std::string>> ShaderLoader::ListAssetResources(
        const std::filesystem::path &p_shaderPath)
    {
        std::vector<std::pair<ResourceType, std::string>> resources;
        
        std::string resourceName = p_shaderPath.stem().string();
        resources.emplace_back(ResourceType::Shader, resourceName);
        return resources;
    }
}
