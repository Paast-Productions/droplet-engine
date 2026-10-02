#include "SceneSerializer.hpp"


using namespace Droplet::Serializer;
using namespace Droplet;

nlohmann::json SceneSerializer::SerializeScene([[maybe_unused]] const Scene::Scene& p_scene)
{
    nlohmann::json json;

    json["version"] = 1; // Should we even have version?, shouldn't be implemented like this tho


    json["name"] = p_scene.GetName();
    json["roots"] = nlohmann::json::array();

    for (const auto& root : p_scene.GetRoots())
    {
        json["roots"].push_back(SerializeNode(*root));
    }

    return json;
}

void SceneSerializer::DeserializeScene([[maybe_unused]] const nlohmann::json& json, [[maybe_unused]] const Scene::Scene& p_scene)
{

}


nlohmann::json SceneSerializer::SerializeNode([[maybe_unused]]const Scene::Node& p_node)
{
    nlohmann::json json;

    json["name"] = p_node.GetName();

    json["transform"] = SerializeTransform(p_node.GetTransform());

    json["components"] = nlohmann::json::array();
    std::unordered_map<std::string, std::function<std::shared_ptr<Scene::Component>()>> registry = Scene::ComponentRegistry::GetRegistry();
    
    for (const auto& componentType : registry)
    {
        
    }

    json["children"] = nlohmann::json::array();

    for (const auto& child : p_node.GetChildren())
    {
        json["children"].push_back(SerializeNode(*child));
    }

    
    return nlohmann::json();
}

void SceneSerializer::DeserializeNode([[maybe_unused]] const nlohmann::json& p_json, [[maybe_unused]] const Scene::Node& p_node)
{

}

nlohmann::json SceneSerializer::SerializeComponent([[maybe_unused]] const Scene::Component& p_component)
{
    std::unordered_map<std::string, std::function<std::shared_ptr<Scene::Component>()>> registry = Scene::ComponentRegistry::GetRegistry();
    nlohmann::json json;


    return json;
}

void SceneSerializer::DeserializeComponent([[maybe_unused]] const nlohmann::json& p_json, [[maybe_unused]] const Scene::Component& p_component)
{

}

nlohmann::json Serializer::SceneSerializer::SerializeTransform([[maybe_unused]] const Scene::Transform& p_transform)
{
    nlohmann::json json;
    

    return json;
}

void Serializer::SceneSerializer::DeserializeTransform([[maybe_unused]] const nlohmann::json& p_json, [[maybe_unused]] const Scene::Transform& p_transform)
{
}
