#include "TriggerComponent.hpp"
#include <Node.hpp>
#include <Transform.hpp>
#include <iostream>

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
            std::cout << "no ";
            continue;
        }
        std::cout << "I AM TRIGGERED!" << std::endl;
    }
    
    // if (!insideNodes.empty())
    // {
    //     std::cout << "I AM TRIGGERED!" << std::endl;
    // }
    // else
    // {
    //     std::cout << "I AM NOT TRIGGERED!" << std::endl;
    // }
    // std::cout << "x: " << bounds->GetTransformedOBB(worldMatrix).center.x << 
    //     "y: " << bounds->GetTransformedOBB(worldMatrix).center.y << 
    //     "z: " << bounds->GetTransformedOBB(worldMatrix).center.z << std::endl;
    // Get bounds
    // Check bounds with another bounds
    // Get other components
    // Trigger script component
    
}
