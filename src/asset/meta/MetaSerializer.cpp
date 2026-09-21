#include "asset/meta/MetaSerializer.hpp"

#include <fstream>

using json = nlohmann::json;

namespace Droplet
{
    bool MetaSerializer::Read(const std::filesystem::path &p_metaFilePath, std::vector<ResourceMetaData> &p_metaData)
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
                for (const auto& body : j["resources"])
                {
                    ResourceMetaData resourceData;
                    resourceData.guid = j.value("guid", C_INVALID_GUID);
                    resourceData.type = static_cast<ResourceType>(j.value("type", static_cast<uint8_t>(ResourceType::None)));
                    resourceData.name = j.value("name", std::string{});
                    resourceData.loadFlags = static_cast<ResourceLoadFlag>(j.value("loadFlags", static_cast<uint8_t>(ResourceLoadFlag::LoadBoth)));
                    resourceData.dependencies = j.value("dependencies", std::vector<GUID>{});
                    resourceData.typeSpecificData = j.value("specificData", json::object());
                    
                    p_metaData.push_back(resourceData);
                }
            }
        }
        catch (const std::exception &e)
        {
            // TODO: Log exception
            return false;
        }

        return true;
    }

    bool MetaSerializer::Write(const std::filesystem::path &p_metaFilePath, std::vector<ResourceMetaData> &p_metaData)
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
            j["loadFlags"] = static_cast<uint8_t>(resourceData.loadFlags);
            j["dependencies"] = resourceData.dependencies;
            j["specificData"] = resourceData.typeSpecificData;
            
            resourcesArray.push_back(j);
        }
        
        root["resources"] = resourcesArray;
        
        file << root.dump(4);
        return true;
    }

    std::string MetaSerializer::GetLowercaseExtension(const std::filesystem::path &p_filePath)
    {
        std::string ext = p_filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
        
        return ext;
    }

    bool MetaSerializer::GenerateDefaultMetaFile(const std::filesystem::path &p_assetPath, std::vector<ResourceMetaData> &p_metaData)
    {
        p_metaData.clear();
        
        ResourceMetaData defaultData;
        defaultData.guid = GuidUtils::Generate();
        defaultData.type = ResourceType::None;
        defaultData.name = std::string{};
        defaultData.loadFlags = ResourceLoadFlag::LoadBoth;
        defaultData.dependencies = std::vector<GUID>{};
        
        std::string ext = GetLowercaseExtension(p_assetPath);
        
        // Models & Meshes
        if (ext == ".fbx" || ext == ".gltf" || ext == ".obj")
        {
            defaultData.typeSpecificData["scale"] = C_MODEL_DEFAULT_SCALE;
            defaultData.typeSpecificData["import_animations"] = C_MODEL_DEFAULT_IMPORT_ANIMATIONS;
        }
        else if (ext == ".dds" || ext == ".ktx")
        {
            // defaultMeta.typeSpecificData["..."] = ...;
        }
        
        p_metaData.push_back(defaultData);
        if (!Write(p_assetPath.string() + ".meta", p_metaData))
        {
            // TODO: Log write error
            return false;
        }
        
        return true;
    }
}
