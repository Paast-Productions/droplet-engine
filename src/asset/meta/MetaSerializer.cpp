#include "asset/meta/MetaSerializer.hpp"

using json = nlohmann::json;

namespace Droplet
{
    ResourceMetaData MetaSerializer::Read(const std::filesystem::path &p_metaFilePath)
    {

    }

    bool MetaSerializer::Read(const std::filesystem::path &p_metaFilePath, ResourceMetaData p_metaData)
    {
        std::ifstream file(p_metaFilePath);
        if (!file.is_open())
        {
            return false;
        }

        try
        {
            json j;
            file >> j;
            
            p_metaData.guid = j.value("guid", C_INVALID_GUID);
            p_metaData.loadFlags = static_cast<AssetLoadFlag>(j.value("loadFlags", static_cast<uint8_t>(AssetLoadFlag::LoadBoth)));
            p_metaData.dependencies = j.value("dependencies", std::vector<std::string>{});
            p_metaData.typeSpecificData = j.value("specificData", json::object());
        }
        catch (const std::exception &e)
        {
            return false;
        }

        return true;
    }

    bool MetaSerializer::Write(const std::filesystem::path &p_metaFilePath, const ResourceMetaData &p_metaData)
    {
        std::ofstream file(p_metaFilePath);
        if (file.is_open()) return false;
        
        json j;
        j["guid"] = p_metaData.guid;
        j["loadFlags"] = static_cast<uint8_t>(p_metaData.loadFlags);
        j["dependencies"] = p_metaData.dependencies;
        j["specificData"] = p_metaData.typeSpecificData;
        
        file << j.dump(4);
        return true;
    }

    std::string MetaSerializer::GetLowercaseExtension(const std::filesystem::path &p_filePath)
    {
        std::string ext = p_filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){return std::tolower(c);});
        
        return ext;
    }

    bool MetaSerializer::GenerateDefaultMetaFile(const std::filesystem::path &p_assetPath, ResourceMetaData &p_metaData)
    {
        p_metaData.guid = GuidUtils::Generate();
        p_metaData.loadFlags = AssetLoadFlag::LoadBoth;
        
        std::string ext = GetLowercaseExtension(p_assetPath);
        
        // 3D model assets (these are the asset types we support)
        if (ext == ".fbx" || ext == ".gltf" || ext == ".obj")
        {
            p_metaData.typeSpecificData["scale"] = 1.0f;
            p_metaData.typeSpecificData["import_animations"] = false;
        }
    }
}
