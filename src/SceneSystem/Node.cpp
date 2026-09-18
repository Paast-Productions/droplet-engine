#include "Node.hpp"
#include "Scene.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Component.hpp"

Node::Node(std::string p_name)
    : m_name(std::move(p_name))
{
}

void Node::OnStart()
{
    if (!m_active || m_started)
    {
        return;
    }

    m_started = true;

    for (const auto& component : m_components)
    {
        component->OnStart();
    }

    for (const auto& child : m_children)
    {
        child->OnStart();
    }
}

void Node::Update(float p_deltaTime)
{
    if (!m_active)
    {
        return;
    }

    //update transforms :=

    for (const auto& component : m_components)
    {
        component->Update(p_deltaTime);
    }

    for (const auto& child : m_children)
    {
        child->Update(p_deltaTime);
    }


}
void Node::Render()
{
    if (!m_active)
    {
        return;
    }

    return;
}

void Node::SetActive(bool p_active)
{
    if (m_active == p_active)
    {
        return;
    }

    m_active = p_active;

    if (m_active)
    {
        if (auto scene = m_scene.lock())
        {
            if (scene->IsActive())
            {
                OnStart();
            }
        }
    }
}

bool Node::IsActive() const
{
    return m_active;
}

std::shared_ptr<Node> Node::AddChild(std::shared_ptr<Node> p_child)
{
    if (!p_child)
    {
        return nullptr;
    }

    p_child->m_parent = shared_from_this();

    if (auto scene = m_scene.lock())
    {
        p_child->SetScene(scene);
    }

    m_children.push_back(p_child);

    if (m_started)
    {
        p_child->OnStart();
    }

    return p_child;
}

void Node::RemoveChild(std::shared_ptr<Node> p_child)
{
    auto it = std::find(m_children.begin(), m_children.end(), p_child);

    if (it == m_children.end())
    {
        return;
    }

    (*it)->m_parent.reset();

    m_children.erase(it);
}

const std::string& Node::GetName() const
{
    return m_name;
}

const std::vector<std::shared_ptr<Node>>& Node::GetChildren() const
{
    return m_children;
}

std::shared_ptr<Node> Node::GetParent() const
{
    return m_parent.lock();
}

void Node::SetPosition(const glm::vec3& p_position)
{
    m_position = p_position;
}

void Node::SetRotation(const glm::vec3& p_rotation)
{
    m_rotation = p_rotation;
}

void Node::SetScale(const glm::vec3& p_scale)
{
    m_scale = p_scale;
}

const glm::vec3& Node::GetPosition() const
{
    return m_position;
}

const glm::vec3& Node::GetRotation() const
{
    return m_rotation;
}

const glm::vec3& Node::GetScale() const
{
    return m_scale;
}

const glm::mat4& Node::GetLocalTransform() const
{
    return m_localTransform;
}

const glm::mat4& Node::GetWorldTransform() const
{
    return m_worldTransform;
}
  