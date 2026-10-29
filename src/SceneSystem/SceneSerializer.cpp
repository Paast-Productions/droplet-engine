#include "SceneSerializer.hpp"
#include "ComponentFactory.hpp"
#include "SceneSystem/Serialization/SerializationContext.hpp"

#include <tracy/public/tracy/Tracy.hpp>

using namespace Droplet::Scene;
using namespace Droplet;

nlohmann::json SceneSerializer::SerializeScene( const std::shared_ptr<Scene> p_scene)
{
    ZoneScoped;
	ZoneText(p_scene->GetName().c_str(), p_scene->GetName().size());

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

void SceneSerializer::DeserializeScene(const nlohmann::json &p_json,std::shared_ptr<Scene> p_scene)
{
    ZoneScoped;
	ZoneText(p_json["name"].get<std::string>().c_str(), p_json["name"].get<std::string>().size());

    SerializationContext context;
    if (!p_scene)
    {
        std::cerr << "Failed to load Game scene\n";
        throw std::runtime_error("Cannot load Scene '" + p_json["name"]);
    }

    for (const auto &rootJson : p_json["roots"])
    {
        auto root = p_scene->AddNode(rootJson["name"]);
        context.RegisterNode(rootJson["id"], root);
        DeserializeTransform(rootJson["transform"], root->GetTransform());
        for (const auto componentJson : rootJson["components"])
        {
            DeserializeComponent(componentJson, root, rootJson["id"], context);
        }
        for (const auto childJson : rootJson["children"])
        {
            DeserializeNode(childJson, root, p_scene,context);
        }
    }
    context.ResolveAllReferences();
}

void Droplet::Scene::SceneSerializer::DeserializeLonsesomeNode(const nlohmann::json &p_json, Droplet::Scene::Scene& p_scene, std::optional<glm::vec3> p_overridePosition)
{
    if (!p_json.contains("name"))
    {
        throw std::runtime_error("Node is missing required field 'name");
    }

    if (!p_json.contains("components"))
    {
        throw std::runtime_error("Node is missing required field 'components'");
    }

    if (!p_json.contains("transform"))
    {
        throw std::runtime_error("Node is missing required field 'transform'");
    }

    if (!p_json.contains("children"))
    {
        throw std::runtime_error("Node is missing required field 'name'");
    }

    if (!p_json.contains("id"))
    {
        throw std::runtime_error("Node is missing required field 'id'");
    }

    if (!p_json.contains("active"))
    {
        throw std::runtime_error("Node is missing required field 'active'");
    }
    auto node = p_scene.AddNode(p_json["name"]);
    SerializationContext nullContext;
    
    DeserializeTransform(p_json["transform"], node->GetTransform());
    if (p_overridePosition.has_value())
    {
        node->GetTransform().SetPosition(*p_overridePosition);
    }


    std::shared_ptr<Droplet::Scene::Scene> scenePtr = p_scene.shared_from_this();

    for (const auto componentJson : p_json["components"])
    {
        DeserializeComponent(componentJson, node, p_json["id"], nullContext);
    }
    for (const auto childJson : p_json["children"])
    {
        DeserializeNode(childJson, node, scenePtr, nullContext);
    }
}


nlohmann::json SceneSerializer::SerializeNode(const Node &p_node)
{
    nlohmann::json json;
    
    json["name"] = p_node.GetName();
    json["id"] = p_node.GetID();
    json["active"] = p_node.IsActive();
	json["drawLayer"] = static_cast<uint16_t>(p_node.GetDrawLayer());
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

void SceneSerializer::DeserializeNode( const nlohmann::json& p_json, std::shared_ptr<Node> p_parentNode, std::shared_ptr<Droplet::Scene::Scene> p_scene, SerializationContext& p_context)
{
    if (!p_json.contains("name"))
    {
        throw std::runtime_error("Node is missing required field 'name'");
    }

    if (!p_json.contains("components"))
    {
        throw std::runtime_error("Node is missing required field 'components'");
    }

    if (!p_json.contains("transform"))
    {
        throw std::runtime_error("Node is missing required field 'transform'");
    }

    if (!p_json.contains("children"))
    {
        throw std::runtime_error("Node is missing required field 'name'");
    }

    if (!p_json.contains("id"))
    {
        throw std::runtime_error("Node is missing required field 'id'");
    }

    if (!p_json.contains("active"))
    {
        throw std::runtime_error("Node is missing required field 'active'");
    }

    

    auto node = p_parentNode->AddChild(p_scene->AddNode(p_json["name"]));
    p_context.RegisterNode(p_json["id"], node);
    node->SetActive(p_json["active"]);

    if (p_json.contains("drawLayer"))
    {
		uint16_t drawLayerValue = p_json["drawLayer"];
		node->SetDrawLayer(static_cast<Graphics::DrawLayer>(drawLayerValue));
    }
    
    DeserializeTransform(p_json["transform"], node->GetTransform());

    for (const auto &componentJson : p_json["components"])
    {
        DeserializeComponent(componentJson, node, p_json["id"], p_context);

    }

    for (const auto &childJson : p_json["children"])
    {
        DeserializeNode(childJson, node, p_scene, p_context);
    }
}

void Droplet::Scene::SceneSerializer::DeserializeComponent(const nlohmann::json &p_json, std::shared_ptr<Droplet::Scene::Node> p_node, std::uint64_t p_serializedNodeID,SerializationContext& p_context)
{
    if (!p_json.contains("type"))
    {
        throw std::runtime_error("Component is missing required field 'type'");
    }
    if (!p_json.contains("id"))
    {
        throw std::runtime_error("Component is missing required field 'id'");
    }
    if (!p_json.contains("data"))
    {
        throw std::runtime_error("Component is missing required field 'data'");
    }
    std::string type = p_json["type"];
    auto component = ComponentFactory::CreateComponent(type);
    std::shared_ptr<Component> sharedComponent(std::move(component));
    p_node->AttachComponent(sharedComponent, p_json["id"]);
    sharedComponent->Deserialize(p_json);

    std::uint64_t ID = p_json["id"];
    p_context.RegisterCopmonent(p_serializedNodeID, ID, sharedComponent);

}

nlohmann::json SceneSerializer::SerializeTransform( const Transform &p_transform)
{
    nlohmann::json json;
    
    json["position"] = { p_transform.GetPosition().x, p_transform.GetPosition().y, p_transform.GetPosition().z };

    json["rotation"] = { p_transform.GetRotation().w , p_transform.GetRotation().x, p_transform.GetRotation().y, p_transform.GetRotation().z };

    json["scale"] = { p_transform.GetScale().x, p_transform.GetScale().y, p_transform.GetScale().z };

    return json;
}

void SceneSerializer::DeserializeTransform(const nlohmann::json& p_json, Transform& p_transform)
{
    if (!p_json.contains("scale"))
    {
        throw std::runtime_error("transform is missing required field 'scale'");
    }

    if (!p_json.contains("rotation"))
    {
        throw std::runtime_error("transform is missing required field 'rotation'");
    }

    if (!p_json.contains("position"))
    {
        throw std::runtime_error("transform is missing required field 'position'");
    }

    glm::vec3 position = glm::vec3(p_json["position"][0], p_json["position"][1], p_json["position"][2]);

    //TODO make sure glm reads w x y z or x y z w
    glm::quat rotation = glm::quat(p_json["rotation"][0], p_json["rotation"][1], p_json["rotation"][2], p_json["rotation"][3]);

    glm::vec3 scale = glm::vec3(p_json["scale"][0], p_json["scale"][1], p_json["scale"][2]);

    p_transform.SetPosition(position);
    p_transform.SetRotation(rotation);
    p_transform.SetScale(scale);
}
