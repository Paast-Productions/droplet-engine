#include "TriggerComponent.hpp"
#include <Node.hpp>
#include <Transform.hpp>
#include <Components/ScriptComponent.hpp>
#include <Time.hpp>
#include <Graphics/DebugDraw.hpp>
#include <SceneSystem/Scene.hpp>

using namespace Droplet::Scene;
using namespace Droplet::Math;

void TriggerComponent::StartImpl()
{
    
}

void TriggerComponent::UpdateImpl()
{
    
}

void TriggerComponent::FixedUpdateImpl()
{
    m_isColliding = false;
    m_collidingNodes.clear();
    
    std::shared_ptr<Node> node = GetOwner();
    std::shared_ptr<Scene> scene = node->GetScene();
    std::vector<std::weak_ptr<Node>> insideNodes;
    for (const AABB &aabb : m_AABBs)
    {
        scene->GetNodesInShape(aabb, insideNodes);
    }
    
    for (const OBB &obb : m_OBBs)
    {
        scene->GetNodesInShape(obb, insideNodes);
    }
    
    for (const Sphere &sphere : m_spheres)
    {
        scene->GetNodesInShape(sphere, insideNodes);
    }
    
    // Remove duplicated nodes
    std::vector<std::weak_ptr<Node>> uniqueNodes;
    for (const std::weak_ptr<Node> &node : insideNodes)
    {
        auto nodePtr = node.lock();
        if (nodePtr == nullptr)
        {
            continue;
        }
			
        bool exists = std::ranges::any_of(uniqueNodes,
            [&nodePtr](const std::weak_ptr<Node> &existing)
            {
                return existing.lock() == nodePtr;
            });
			
        if (!exists)
        {
            uniqueNodes.push_back(node);
        }
    }
    
    // Run scripts attached to nodes triggering TriggerComponent
    for (const auto &weakNode : uniqueNodes)
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
            Script::ScriptSystem::Get().TriggerComponentScript(script.get(), Droplet::Time::Get().GetFixedDeltaTime(), otherNode);
        }
    }
}

void TriggerComponent::RenderUIImpl()
{
    if (m_AABBs.empty() && m_OBBs.empty() && m_spheres.empty())
    {
        ImGui::TextWrapped("TriggerComponent currently has no trigger shape!");
    }
    
    static bool drawShapes = true;
    ImGui::Checkbox("Draw trigger shapes", &drawShapes);
    if (drawShapes)
    {
        for (AABB aabb : m_AABBs)
        {
            Graphics::Debug::DebugDraw::DrawBox(aabb, glm::vec4(1.0, 0.0, 0.0f, 1.0f), true, true);
        }
    
        for (OBB obb : m_OBBs)
        {
            Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(1.0, 0.0, 0.0f, 1.0f), true, true);
        }
    
        for (Sphere sphere : m_spheres)
        {        
            Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, 0.0f), sphere.radius, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 16, true, true);
            Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(0.0f, 1.0f, 0.0f), sphere.radius, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 16, true, true);
            Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(0.0f, 0.0f, 1.0f), sphere.radius, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 16, true, true);
            Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, 1.0f), sphere.radius, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 16, true, true);
            Graphics::Debug::DebugDraw::DrawCircle(sphere.center, glm::vec3(1.0f, 0.0f, -1.0f), sphere.radius, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 16, true, true);
        }
    }
    
    static bool drawCollidingShapes = true;
    ImGui::Checkbox("Draw colliding shapes", &drawCollidingShapes);
    if (m_isColliding && drawCollidingShapes)
    {
        for (AABB aabb : m_AABBs)
        {
            Graphics::Debug::DebugDraw::DrawBox(aabb, glm::vec4(1.0, 0.0, 0.0f, 0.25f), true, false);
        }
    
        for (OBB obb : m_OBBs)
        {
            Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(1.0, 0.0, 0.0f, 0.25f), true, false);
        }
    
        for (Sphere sphere : m_spheres)
        {
            Graphics::Debug::DebugDraw::DrawSphere(sphere,glm::vec4(1.0, 0.0, 0.0f, 0.25f), 1, false);
        }
        
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
            const OBB& otherObb = otherBounds->GetTransformedOBB(otherVM);
            Graphics::Debug::DebugDraw::DrawBox(otherObb, glm::vec4(0.0 ,0.0,1.0f,1.0f), true, true );
        }
    }
    
    const char *shapes[] = { "AABB", "OBB", "Sphere" };
    static int currentShape = 0;
    ImGui::Combo("Shape", &currentShape, shapes, IM_COUNTOF(shapes));
    
    static glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
    static glm::vec3 ext = glm::vec3(0.5f, 0.5f, 0.5f);
    static glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    static float r = 0.5f;
    
    static AABB aabb = AABB(pos, ext); 
    static OBB obb = OBB(pos, ext, glm::quat(rot));
    static Sphere sphere = Sphere(pos, r); 
    
    ImGui::DragFloat3("Position", &pos.x, 0.1f);
    switch (currentShape)
    {
    case 0: // AABB
        ImGui::DragFloat3("Extents", &ext.x, 0.1f);
        aabb.center = pos;
        aabb.extents = ext;
        Graphics::Debug::DebugDraw::DrawBox(aabb, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), true, true);
        break;
    case 1: // OBB
        ImGui::DragFloat3("Extents", &ext.x, 0.1f);
        ImGui::DragFloat3("Rotation", &rot.x, 0.1f);
        obb.center = pos;
        obb.extents = ext;
        obb.orientation = glm::mat3_cast(glm::quat(rot));
        Graphics::Debug::DebugDraw::DrawBox(obb, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), true, true);
        break;
    case 2: // Sphere
        ImGui::DragFloat("Radius", &r, 0.1f);
        sphere.center = pos;
        sphere.radius = r;
        Graphics::Debug::DebugDraw::DrawCircle(pos, glm::vec3(1.0f, 0.0f, 0.0f), r, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 16, true, true);
        Graphics::Debug::DebugDraw::DrawCircle(pos, glm::vec3(0.0f, 1.0f, 0.0f), r, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 16, true, true);
        Graphics::Debug::DebugDraw::DrawCircle(pos, glm::vec3(0.0f, 0.0f, 1.0f), r, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 16, true, true);
        Graphics::Debug::DebugDraw::DrawCircle(pos, glm::vec3(1.0f, 0.0f, 1.0f), r, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 16, true, true);
        Graphics::Debug::DebugDraw::DrawCircle(pos, glm::vec3(1.0f, 0.0f, -1.0f), r, glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), 16, true, true);
        break;
    default:
    }
    
    if (ImGui::Button("Add shape"))
    {
        switch (currentShape)
        {
        case 0: // AABB
            m_AABBs.push_back(aabb);
            break;
        case 1: // OBB
            m_OBBs.push_back(obb);
            break;
        case 2: // Sphere
            m_spheres.push_back(sphere);
            break;
        default:
        }
    }
}
