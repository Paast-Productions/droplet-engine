#include "resource/meta/MetaUtils.hpp"
#include "resource/types/ShaderResource.hpp"

#include <fstream>

using json = nlohmann::json;

namespace Droplet
{
    bool MetaUtils::Read(const std::filesystem::path &p_metaFilePath, std::vector<MetaEntry> &p_metaData)
    {
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
                    resourceData.name = entry.value("name", std::string{});
                    resourceData.assetPath = entry.value("path", std::string{});
                    resourceData.loadFlags = static_cast<ResourceLoadFlag>(entry.value("loadFlags", static_cast<uint8_t>(ResourceLoadFlag::LoadCPU)));
                    resourceData.dependencies = entry.value("dependencies", std::vector<GUID>{});
                    resourceData.typeSpecificData = entry.value("specificData", json::object());
                    
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

    bool MetaUtils::Write(const std::filesystem::path &p_metaFilePath, std::vector<MetaEntry> &p_metaData)
    {
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
            j["path"] = resourceData.assetPath;
            j["loadFlags"] = static_cast<uint8_t>(resourceData.loadFlags);
            j["dependencies"] = resourceData.dependencies;
            j["specificData"] = resourceData.typeSpecificData;
            
            resourcesArray.push_back(j);
        }
        
        root["resources"] = resourcesArray;
        
        file << root.dump(4);
        return true;
    }

    ShaderResource::ShaderType MetaUtils::EvaluateShaderTypeFromPath(const std::filesystem::path &p_shaderPath)
    {
        std::string shaderFileName = p_shaderPath.filename().generic_string();
        std::transform(shaderFileName.begin(), shaderFileName.end(), shaderFileName.begin(), [](unsigned char c)
        {
            return tolower(c);
        });
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

        return ShaderResource::ShaderType::Vertex; // Default to vertex
    }

    MetaEntry MetaUtils::GenerateDefaultMetaEntry(ResourceType p_type, const std::string &p_name, const std::string &p_assetPath)
    {
        MetaEntry entry;
        entry.guid = GuidUtils::Generate();
        entry.type = p_type;
        entry.name = p_name;
        entry.assetPath = p_assetPath;
        
        // Set type-specific flags
        switch (p_type)
        {
        case ResourceType::Texture2D:
        case ResourceType::Texture3D:
            entry.loadFlags = ResourceLoadFlag::LoadGPU;
            entry.typeSpecificData["generate_mipmaps"] = C_TEXTURE_DEFAULT_GENERATE_MIPMAPS;
            break;
        case ResourceType::Mesh:
        case ResourceType::SkinnedMesh:
            entry.loadFlags = ResourceLoadFlag::LoadBoth;
            entry.typeSpecificData["generate_normals"] = C_MESH_DEFAULT_IMPORT_GENERATE_NORMALS;
            entry.typeSpecificData["join_identical_vertices"] = C_MESH_DEFAULT_IMPORT_JOIN_IDENTICAL_VERTICES;
            entry.typeSpecificData["triangulate"] = C_MESH_DEFAULT_IMPORT_TRIANGULATE;
            break;
        case ResourceType::Animation:
            entry.loadFlags = ResourceLoadFlag::LoadCPU;
            break;
        case ResourceType::Shader:
            entry.loadFlags = ResourceLoadFlag::LoadGPU;
            
            entry.typeSpecificData["shader_type"] = EvaluateShaderTypeFromPath(p_assetPath);
            break;
        case ResourceType::Material:
            entry.loadFlags = ResourceLoadFlag::LoadCPU;
            break;
        default: 
            break;
        }
        
        return entry;
    }
}
