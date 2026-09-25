#include "resource/loaders/SlangLoader.hpp"

#include "graphics/vk/ShaderCompiler.hpp"
#include "resource/meta/MetaUtils.hpp"

using json = nlohmann::json;

namespace Droplet::SlangLoader
{
    std::unique_ptr<ShaderResource> CompileAndLoad(const std::filesystem::path &p_shaderPath, const json &p_loadSettings)
    {
        ShaderResource::ShaderType type = p_loadSettings.value("shader_type", MetaUtils::C_SHADER_DEFAULT_TYPE);
        
        try
        {
            thread_local Graphics::ShaderCompiler s_compiler;
            return std::make_unique<ShaderResource>(type, s_compiler.CompileShader(p_shaderPath));
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
