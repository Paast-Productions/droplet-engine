#include "SceneSerializer.hpp"


using namespace Droplet::Scene;
using namespace Droplet;

nlohmann::json SceneSerializer::SerializeScene( const std::shared_ptr<Scene> p_scene)
{
    nlohmann::json json;

    json["version"] = 1; // Should we even have version?, shouldn't be implemented like this tho


    json["name"] = p_scene->GetName();
    json["roots"] = nlohmann::json::array();

    for (const auto& root : p_scene->GetRoots())
    {
        json["roots"].push_back(SerializeNode(*root));
    }
    
    return json;
}

void SceneSerializer::DeserializeScene([[maybe_unused]] const nlohmann::json& json, [[maybe_unused]] Scene& p_scene)
{

}


nlohmann::json SceneSerializer::SerializeNode(const Node& p_node)
{
    nlohmann::json json;

    
    json["name"] = p_node.GetName();
    json["transform"] = SerializeTransform(p_node.GetTransform());

    json["components"] = nlohmann::json::array();
    
    for (const auto& component : p_node.GetAllComponents())
    {
        json["components"].push_back(component->Serialize());
    }

    json["children"] = nlohmann::json::array();

    for (const auto& child : p_node.GetChildren())
    {
        json["children"].push_back(SerializeNode(*child));
    }

  

    return json;
}

void SceneSerializer::DeserializeNode([[maybe_unused]] const nlohmann::json& p_json, [[maybe_unused]] Node& p_parentNode, std::shared_ptr<Droplet::Scene::Scene> p_scene)
{
    auto node = p_parentNode.AddChild(p_scene->AddNode(p_json["name"]));
    
    DeserializeTransform(p_json["transform"], node->GetTransform());

    for (const auto &componentJson : p_json["components"])
    {
        std::string type = componentJson["type"];

        if (type == "ScriptComponent")
        {
            //node->AddComponent<ScriptComponent>()
        }
        else if (type == "MeshComponent")
        {

        }
        else if (type == "PlayerComponent")
        {

        }
        else
        {
            
        }
    }
}


nlohmann::json SceneSerializer::SerializeTransform( const Transform& p_transform)
{
    nlohmann::json json;
    
    json["position"] = { p_transform.GetPosition().x, p_transform.GetPosition().y, p_transform.GetPosition().z };

    json["rotation"] = { p_transform.GetRotation().w , p_transform.GetRotation().x, p_transform.GetRotation().y, p_transform.GetRotation().z };

    json["scale"] = { p_transform.GetScale().x, p_transform.GetScale().y, p_transform.GetScale().z };

    return json;
}

void SceneSerializer::DeserializeTransform([[maybe_unused]] const nlohmann::json& p_json, Transform& p_transform)
{
    glm::vec3 position = glm::vec3(p_json["position"][0], p_json["position"][1], p_json["position"][2]);

    glm::quat rotation = glm::quat(p_json["rotation"][0], p_json["rotation"][1], p_json["rotation"][2], p_json["rotation"][3]);

    glm::vec3 scale = glm::vec3(p_json["scale"][0], p_json["scale"][1], p_json["scale"][2]);

    p_transform.SetPosition(position);
    p_transform.SetRotation(rotation);
    p_transform.SetScale(scale);

}
