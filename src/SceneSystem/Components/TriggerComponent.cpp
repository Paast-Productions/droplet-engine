#include "TriggerComponent.hpp"
#include <Node.hpp>
#include <Transform.hpp>
#include <Components/ScriptComponent.hpp>
#include <iostream>
#include <Time.hpp>

#include "Scene.hpp"
using namespace Droplet::Scene;

void TriggerComponent::RenderUIImpl()
{
    ImGui::Text("TriggerComponent");
}

void TriggerComponent::StartImpl()
{
    
}

void TriggerComponent::FixedUpdateImpl()
{
    // Get node
    std::shared_ptr<Node> node = GetOwner();
    
    glm::mat4 worldMatrix = node->GetTransform().GetMatrix(Transform::Space::World);
    
    const NodeBounds* bounds = node->GetBounds();
    
    const Droplet::Math::OBB& obb = bounds->GetTransformedOBB(worldMatrix);
    
    std::vector<std::weak_ptr<Node>> insideNodes = node->GetScene()->GetNodesInBoundingBox(obb);
        
    for (const auto &weakNode : insideNodes)
    {
        auto otherNode = weakNode.lock();
        
        if (!otherNode || otherNode == node)
        {
            continue;
        }
        std::vector<std::shared_ptr<ScriptComponent>> scripts = node->GetComponents<ScriptComponent>(); 
        
        for (const auto &script : scripts)
        {
            ScriptSystem::Get().TriggerComponentScript(script.get(), Droplet::Time::Get().GetDeltaTime());
        }
        std::cout << scripts.size() << std::endl;
    
    }
}
