#include "Node.hpp"

void Node::Update(float p_deltaTime)
{
    if (!m_active)
    {
        return;
    }

    for (const auto& child : m_children)
    {
        child->Update(p_deltaTime);
    }
}

std::shared_ptr<Node> Node::AddChild(std::shared_ptr<Node> p_child)
{
    if (!p_child)
    {
        return nullptr;
    }

    p_child->m_parent = shared_from_this();

    m_children.push_back(p_child);

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
  