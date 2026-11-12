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
    for (const TriggerAABB &aabb : m_AABBs)
    {
        scene->GetNodesInShape(aabb.aabb, insideNodes);
    }
    
    for (const TriggerOBB &obb : m_OBBs)
    {
        scene->GetNodesInShape(obb.obb, insideNodes);
    }
    
    for (const TriggerSphere &sphere : m_spheres)
    {
        scene->GetNodesInShape(sphere.sphere, insideNodes);
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
    
    // Add shapes
    static int currentShape = 0;

    AddShape(currentShape);
    int selectedIdx = ListShapes(currentShape);
    
    // Draw wireframes of all shapes
    static bool drawShapes = true;
    ImGui::Checkbox("Draw trigger shapes", &drawShapes);
    if (drawShapes)
    {
        for (int i = 0; i < m_AABBs.size(); i++)
        {
            if (currentShape == 0 && i == selectedIdx)
            {
                m_AABBs[i].Draw(glm::vec4(0.0, 1.0, 0.0f, 1.0f));
            }
            else
            {
                m_AABBs[i].Draw(glm::vec4(1.0, 0.0, 0.0f, 1.0f));
            }
        }
    
        for (int i = 0; i < m_OBBs.size(); i++)
        {
            if (currentShape == 1 && i == selectedIdx)
            {
                m_OBBs[i].Draw(glm::vec4(0.0, 1.0, 0.0f, 1.0f));
            }
            else
            {
                m_OBBs[i].Draw(glm::vec4(1.0, 0.0, 0.0f, 1.0f));
            }
        }
    
        for (int i = 0; i < m_spheres.size(); i++)
        {
            if (currentShape == 2 && i == selectedIdx)
            {
                m_spheres[i].Draw(glm::vec4(0.0, 1.0, 0.0f, 1.0f));
            }
            else
            {
                m_spheres[i].Draw(glm::vec4(1.0, 0.0, 0.0f, 1.0f));
            }
        }
    }
    
    // Draw shapes when a node collides with the trigger
    static bool drawCollidingShapes = true;
    ImGui::Checkbox("Draw colliding shapes", &drawCollidingShapes);
    if (m_isColliding && drawCollidingShapes)
    {
        for (const TriggerAABB &aabb : m_AABBs)
        {
            aabb.Draw(glm::vec4(1.0, 0.0, 0.0f, 0.25f), true, false);
        }
    
        for (const TriggerOBB &obb : m_OBBs)
        {
            obb.Draw(glm::vec4(1.0, 0.0, 0.0f, 0.25f), true, false);
        }
    
        for (const TriggerSphere &sphere : m_spheres)
        {
            sphere.Draw(glm::vec4(1.0, 0.0, 0.0f, 0.25f), true, false);
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
    
    // Move all shapes when the trigger node moves
    glm::vec3 nodePos = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
    for (TriggerAABB &aabb : m_AABBs)
    {
        aabb.aabb.center = aabb.offset + nodePos;
    }
    for (TriggerOBB &obb : m_OBBs)
    {
        obb.obb.center = obb.offset + nodePos;
    }
    for (TriggerSphere &sphere : m_spheres)
    {
        sphere.sphere.center = sphere.offset + nodePos;
    }
}

void TriggerComponent::AddShape(int& p_currentShape)
{
    glm::vec3 nodePos = GetOwner()->GetTransform().GetPosition(Transform::Space::World);
    
    const char *shapes[] = { "AABB", "OBB", "Sphere" };
    ImGui::Combo("Shape", &p_currentShape, shapes, IM_COUNTOF(shapes));

    static glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f);
    static glm::vec3 ext = glm::vec3(0.5f, 0.5f, 0.5f);
    static glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f);
    static float r = 0.5f;

    static TriggerAABB newAabb = { { pos }, AABB(pos, ext) };
    static TriggerOBB newObb = { { pos }, OBB(pos, ext, glm::quat(rot)) };
    static TriggerSphere newSphere = { { pos }, Sphere(pos, r) };

    ImGui::DragFloat3("Position", &pos.x, 0.1f);

    switch (p_currentShape)
    {
    case 0: // AABB
        ImGui::DragFloat3("Extents", &ext.x, 0.1f);
        newAabb.offset = pos;
        newAabb.aabb.center = newAabb.offset + nodePos;
        newAabb.aabb.extents = ext;
        newAabb.Draw(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        break;
    case 1: // OBB
        ImGui::DragFloat3("Extents", &ext.x, 0.1f);
        ImGui::DragFloat3("Rotation", &rot.x, 0.1f);
        newObb.offset = pos;
        newObb.obb.center = pos + nodePos;
        newObb.obb.extents = ext;
        newObb.obb.orientation = glm::mat3_cast(glm::quat(rot));
        newObb.Draw(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        break;
    case 2: // Sphere
        ImGui::DragFloat("Radius", &r, 0.1f);
        newSphere.offset = pos;
        newSphere.sphere.center = pos + nodePos;
        newSphere.sphere.radius = r;
        newSphere.Draw(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
        break;
    default:
    }

    if (ImGui::Button("Add shape"))
    {
        switch (p_currentShape)
        {
        case 0: // AABB
            m_AABBs.push_back(newAabb);
            break;
        case 1: // OBB
            m_OBBs.push_back(newObb);
            break;
        case 2: // Sphere
            m_spheres.push_back(newSphere);
            break;
        default:
        }
    }
}

int TriggerComponent::ListShapes(const int p_currentShape)
{
    // List all shapes & remove shapes
    static int returnSelected = -1;
    switch (p_currentShape)
    {
    case 0: // AABB
    {
        static int selected = -1;
        for (int i = 0; i < m_AABBs.size(); i++)
        {
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::Selectable(std::string("AABB##" + std::to_string(i)).c_str(), selected == i))
            {
                selected = i;
                returnSelected = selected;
            }
            if (i == selected)
            {
                m_AABBs[selected].Draw(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false);
            }
            
            ImGui::SameLine();
            if (ImGui::SmallButton(std::string("X##" + std::to_string(i)).c_str()))
            {
                m_AABBs.erase(m_AABBs.begin() + i);
            }
        }
        break;
    }
    case 1: // OBB
    {
        static int selected = -1;
        for (int i = 0; i < m_OBBs.size(); i++)
        {
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::Selectable(std::string("OBB##" + std::to_string(i)).c_str(), selected == i))
            {
                selected = i;
                returnSelected = selected;
            }
            if (i == selected)
            {
                m_OBBs[selected].Draw(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false);
            }
        
            ImGui::SameLine();
            if (ImGui::SmallButton(std::string("X##" + std::to_string(i)).c_str()))
            {
                m_OBBs.erase(m_OBBs.begin() + i);
            }
        }
        break;
    }
    case 2: // Sphere
    {
        static int selected = -1;
        for (int i = 0; i < m_spheres.size(); i++)
        {
            ImGui::SetNextItemAllowOverlap();
            if (ImGui::Selectable(std::string("Sphere##" + std::to_string(i)).c_str(), selected == i))
            {
                selected = i;
                returnSelected = selected;
            }
            if (i == selected)
            {
                m_spheres[selected].Draw(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), false);
            }
        
            ImGui::SameLine();
            if (ImGui::SmallButton(std::string("X##" + std::to_string(i)).c_str()))
            {
                m_spheres.erase(m_spheres.begin() + i);
            }
        }
        break;
    }
    default:
    }
    
    return returnSelected;
}
