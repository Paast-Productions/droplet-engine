#include "resource/meta/MetaUtils.hpp"
#include "resource/meta/MetaData.hpp"
#include "resource/types/ShaderResource.hpp"

#include <fstream>

#include "core/StringUtils.hpp"
#include <tracy/public/tracy/Tracy.hpp>

using json = nlohmann::json;

namespace Droplet::MetaUtils
{
    bool Read(const std::filesystem::path &p_metaFilePath, std::vector<MetaEntry> &p_metaData)
    {
        ZoneScoped;

        std::ifstream file(p_metaFilePath);
        if (!file.is_open())
        {
            // TODO: Log file open error
            return false;
        }

        try
        {
            json j;
            file >> j;
            p_metaData.clear();
            
            if (j.contains("resources") && j["resources"].is_array())
            {
                for (const auto& entry : j["resources"])
                {
                    MetaEntry resourceData;
                    resourceData.guid = entry.value("guid", C_INVALID_GUID);
                    resourceData.type = static_cast<ResourceType>(entry.value("type", static_cast<uint8_t>(ResourceType::None)));
                    resourceData.name = entry.value("name", "");
                    resourceData.loadFlags = static_cast<ResourceLoadFlag>(entry.value("loadFlags", static_cast<uint8_t>(ResourceLoadFlag::LoadCPU)));
                    resourceData.dependencies = entry.value("dependencies", std::vector<GUID>{});
                    resourceData.loadSettings = entry.value("loadSettings", json::object());
                    
                    p_metaData.push_back(resourceData);
                }
            }
        }
        catch (const std::exception&)
        {
            // TODO: Log exception
            return false;
        }

        return true;
    }

    bool Write(const std::filesystem::path &p_metaFilePath, const std::vector<MetaEntry> &p_metaData)
    {
        ZoneScoped;

        std::ofstream file(p_metaFilePath);
        if (!file.is_open())
        {
            // TODO: Log file open error
            return false;
        }
        
        json root;
        json resourcesArray = json::array();
        
        for (const auto& resourceData : p_metaData)
        {
            json j;
            
            j["guid"] = resourceData.guid;
            j["type"] = static_cast<uint8_t>(resourceData.type);
            j["name"] = resourceData.name;
            j["loadFlags"] = static_cast<uint8_t>(resourceData.loadFlags);
            j["dependencies"] = resourceData.dependencies;
            j["loadSettings"] = resourceData.loadSettings;
            
            resourcesArray.push_back(j);
        }
        
        root["resources"] = resourcesArray;
        
        file << root.dump(4);
        return true;
    }

    ShaderResource::ShaderType EvaluateShaderTypeFromPath(const std::filesystem::path &p_shaderPath)
    {
        std::string shaderFileName = p_shaderPath.filename().generic_string();
        StringUtils::ToLowerInPlace(shaderFileName);
        
        if (shaderFileName.starts_with("vs_"))
        {
            return ShaderResource::ShaderType::Vertex;
        }
        if (shaderFileName.starts_with("fs_"))
        {
            return ShaderResource::ShaderType::Fragment;
        }
        if (shaderFileName.starts_with("gs_"))
        {
            return ShaderResource::ShaderType::Geometry;
        }
        if (shaderFileName.starts_with("cs_"))
        {
            return ShaderResource::ShaderType::Compute;
        }
        if (shaderFileName.starts_with("tcs_"))
        {
            return ShaderResource::ShaderType::TessellationControl;
        }
        if (shaderFileName.starts_with("tes_"))
        {
            return ShaderResource::ShaderType::TessellationEvaluation;
        }
        if (shaderFileName.starts_with("ms_"))
        {
            return ShaderResource::ShaderType::Mesh;
        }
        if (shaderFileName.starts_with("ts_"))
        {
            return ShaderResource::ShaderType::Task;
        }

        return MetaLoadSettings::C_SHADER_TYPE.defaultValue;
    }

    MetaEntry GenerateDefaultMetaEntry(ResourceType p_type, const std::string &p_name, const std::string &p_relAssetPath, const json &p_explicitLoadSettings)
    {
        MetaEntry entry;
        entry.guid = GuidUtils::Generate();
        entry.type = p_type;
        entry.name = p_name;
        entry.relAssetPath = p_relAssetPath;
        
        // Set type-specific flags
        switch (p_type)
        {
        case ResourceType::Texture2D:
        case ResourceType::Texture3D:
            entry.loadFlags = ResourceLoadFlag::LoadBoth;
            entry.loadSettings[MetaLoadSettings::C_GENERATE_MIPMAPS.key] = MetaLoadSettings::C_GENERATE_MIPMAPS.defaultValue;
            break;
        case ResourceType::Mesh:
        case ResourceType::SkinnedMesh:
            entry.loadFlags = ResourceLoadFlag::LoadBoth;
            entry.loadSettings[MetaLoadSettings::C_GENERATE_NORMALS.key] = MetaLoadSettings::C_GENERATE_NORMALS.defaultValue;
            entry.loadSettings[MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.key] = MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.defaultValue;
            entry.loadSettings[MetaLoadSettings::C_TRIANGULATE.key] = MetaLoadSettings::C_TRIANGULATE.defaultValue;
            break;
        case ResourceType::Animation:
            entry.loadFlags = ResourceLoadFlag::LoadCPU;
            entry.loadSettings[MetaLoadSettings::C_TARGET_ANIMATION.key] = p_name;
            break;
        case ResourceType::Shader:
            entry.loadFlags = ResourceLoadFlag::LoadBoth;
            entry.loadSettings[MetaLoadSettings::C_SHADER_TYPE.key] = EvaluateShaderTypeFromPath(p_relAssetPath);
            break;
        case ResourceType::Material:
            entry.loadFlags = ResourceLoadFlag::LoadCPU;
            break;
        default: 
            break;
        }
        
        entry.loadSettings.update(p_explicitLoadSettings);
        return entry;
    }

    std::vector<MetaLoadSettings::Descriptor> GetSettingsSchema(ResourceType p_type)
    {
        switch (p_type)
        {
        case ResourceType::Texture2D:
        case ResourceType::Texture3D:
            return {
                {
                    MetaLoadSettings::C_GENERATE_MIPMAPS.key, 
                    MetaLoadSettings::C_GENERATE_MIPMAPS.displayName,
                    MetaLoadSettings::Type::Bool
                }
            };
        case ResourceType::Mesh:
        case ResourceType::SkinnedMesh:
            return {
                {
                    MetaLoadSettings::C_GENERATE_NORMALS.key, 
                    MetaLoadSettings::C_GENERATE_NORMALS.displayName,
                    MetaLoadSettings::Type::Bool
                },
                {
                    MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.key, 
                    MetaLoadSettings::C_JOIN_IDENTICAL_VERTICES.displayName,
                    MetaLoadSettings::Type::Bool
                },
                {
                    MetaLoadSettings::C_TRIANGULATE.key, 
                    MetaLoadSettings::C_TRIANGULATE.displayName,
                    MetaLoadSettings::Type::Bool
                }
            };
        case ResourceType::Animation:
            return {};
        case ResourceType::Shader:
            return {
                {
                    MetaLoadSettings::C_SHADER_TYPE.key,
                    MetaLoadSettings::C_SHADER_TYPE.displayName,
                    MetaLoadSettings::Type::Enum,
                    0.0f,
                    0.0f,
                    {"Vertex", "Fragment", "Geometry", "Compute", "TesselationControl", "TesselationEvaluation", "Mesh", "Task"}
                }
            };
        case ResourceType::Material:
            return {};

        default: 
            return {};
        }
    }
}
