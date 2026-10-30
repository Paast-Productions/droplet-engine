#include "TriggerComponent.hpp"
#include <Node.hpp>
#include <Transform.hpp>
#include <Components/ScriptComponent.hpp>
#include <iostream>
#include <Time.hpp>
#include <Graphics/DebugDraw.hpp>
#include "Scene.hpp"
using namespace Droplet::Scene;

void TriggerComponent::RenderUIImpl()
{
    ImGui::Text("TriggerComponent");
}

void TriggerComponent::StartImpl()
{
    
}

void TriggerComponent::UpdateImpl()
{
   auto node = GetOwner();
    
    if (!node)
    {
        return;
    }
    
    const NodeBounds *bounds = node->GetBounds();
    if (!bounds)
    {
        return;
    }
    
    glm::mat4 worldMatrix = node->GetTransform().GetMatrix(Transform::Space::World);
    
    auto obb = bounds->GetTransformedOBB(worldMatrix);
    
    if (!m_isColliding)
    {
        return;
    }
    
    Droplet::Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(1.0,0.0,0.0f,0.25f), true, false );
    
    for (const auto &weakNode : m_collidingNodes)
    {
        auto otherNode = weakNode.lock();
        if (!otherNode)
        {
            continue;
        }
        
        const NodeBounds *otherBounds = otherNode->GetBounds();
        
        if (!otherBounds)
        {
            continue;
        }
        
        glm::mat4 otherVM = otherNode->GetTransform().GetMatrix(Transform::Space::World);
        const Droplet::Math::OBB& otherObb = otherBounds->GetTransformedOBB(otherVM);
            
        Droplet::Graphics::Debug::DebugDraw::DrawBox(otherObb, glm::vec4(0.0 ,0.0,1.0f,1.0f), true, true );
        
    }
}

void TriggerComponent::FixedUpdateImpl()
{
    m_isColliding = false;
    m_collidingNodes.clear();
    
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
        
        m_isColliding = true;
        m_collidingNodes.push_back(otherNode);
        
        std::vector<std::shared_ptr<ScriptComponent>> scripts = node->GetComponents<ScriptComponent>(); 
        
        for (const auto &script : scripts)
        {
            ScriptSystem::Get().TriggerComponentScript(script.get(), Droplet::Time::Get().GetDeltaTime());
        }
    }
}
