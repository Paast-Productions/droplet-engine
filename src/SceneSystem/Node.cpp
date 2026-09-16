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

std::shared_ptr<Node> Node::AddChild(
    std::shared_ptr<Node> p_child)
{
    if (!p_child)
    {
        return nullptr;
    }

    p_child->m_parent = shared_from_this();

    m_children.push_back(p_child);

    return p_child;
}

void Node::RemoveChild(
    std::shared_ptr<Node> p_child)
{
    auto it = std::find(
        m_children.begin(),
        m_children.end(),
        p_child);

    if (it == m_children.end())
    {
        return;
    }

    (*it)->m_parent.reset();

    m_children.erase(it);
}
  