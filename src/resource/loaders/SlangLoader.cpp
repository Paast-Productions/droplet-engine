#include "resource/loaders/SlangLoader.hpp"

#include "graphics/vk/ShaderCompiler.hpp"

namespace Droplet::SlangLoader
{
    std::unique_ptr<ShaderResource> CompileAndLoad(ShaderResource::ShaderType p_type, const std::filesystem::path &p_shaderPath)
    {
        try
        {
            thread_local Graphics::ShaderCompiler s_compiler;
            return std::make_unique<ShaderResource>(p_type, s_compiler.CompileShader(p_shaderPath));
        }
        catch (const std::exception&)
        {
            // TODO: Log the error
            return nullptr;
        }
    }

    std::vector<std::pair<ResourceType, std::string>> ListAssetResources(
        const std::filesystem::path &p_shaderPath)
    {
        std::vector<std::pair<ResourceType, std::string>> resources{};
        
        std::string resourceName = p_shaderPath.stem().string();
        resources.emplace_back(ResourceType::Shader, resourceName);
        return resources;
    }
}
