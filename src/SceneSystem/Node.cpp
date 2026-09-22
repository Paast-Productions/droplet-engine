#include "Node.hpp"
#include "Scene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Component.hpp"
#include <stdexcept>

using namespace Droplet::Scene;

Node::Node(const std::string &p_name)
	: m_name((p_name)), m_transform(this)
{
}

void Node::Start()
{
    if (!m_active || m_started)
    {
        return;
    }

    m_started = true;

    for (const auto &component : m_components)
    {
        component->Start();
    }

    for (const auto &child : m_children)
    {
        child->Start();
    }
}

void Node::Update(float p_deltaTime)
{
    if (!m_active)
    {
        return;
    }

    //update transforms :=
	//TODO: Implement transform update logic here, including calculating local and world transforms based on position, rotation, and scale.

    for (const auto &component : m_components)
    {
        component->Update(p_deltaTime);
    }

    for (const auto &child : m_children)
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
                Start();
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
        throw std::invalid_argument("Cannot add nullptr as a child Node.");
    }

    if (p_child.get() == this)
    {
        throw std::runtime_error("Cannot add Node '" + m_name + "' as a child of itself.");
    }

    // TODO: Allow reparenting
    if (p_child->GetParent())
    {
        throw std::runtime_error("Cannot add Node '" + p_child->GetName() + "': Node already has a parent.");
    }

	// TODO: Node should already be part of the same Scene at this point
    if (p_child->GetScene())
    {
        throw std::runtime_error("Cannot add Node '" + p_child->GetName() + "': Node already belongs to a Scene.");
    }

    p_child->m_parent = shared_from_this();

    if (auto scene = m_scene.lock())
    {
        p_child->SetScene(scene);
    }

    m_children.push_back(p_child);

    if (m_started)
    {
        p_child->Start();
    }

    return p_child;
}

void Node::RemoveChild(const std::shared_ptr<Node> &p_child)
{
    if (!p_child)
    {
        throw std::invalid_argument("Cannot remove nullptr as a child Node.");
    }

    if (p_child.get()->m_parent.lock() != shared_from_this())
    {
		throw std::runtime_error("Cannot remove Node '" + p_child->GetName() + "': Node is not a child of '" + m_name + "'.");
	}

    auto it = std::find(m_children.begin(), m_children.end(), p_child);

    if (it == m_children.end())
    {
        throw std::runtime_error("Cannot remove Node '" + p_child->GetName() + "': Node is not a child of '" + m_name + "'.");
    }

    p_child->m_parent.reset();
	p_child->SetScene(nullptr); // TODO: Should not be removed from the Scene, but become a root Node.

    m_children.erase(it);
}

void Node::RemoveComponent(const std::shared_ptr<Component> &p_component)
{
    if (!p_component)
    {
        throw std::invalid_argument("Cannot remove nullptr Component.");
    }

    auto it = std::find(m_components.begin(), m_components.end(), p_component);

    if (it == m_components.end())
    {
        throw std::runtime_error("Cannot remove Component: Component is not attached to Node '" + m_name + "'.");
    }

    m_components.erase(it);
}

const std::string& Node::GetName() const
{
    return m_name;
}

const std::vector<std::shared_ptr<Node>> &Node::GetChildren() const
{
    return m_children;
}

std::shared_ptr<Node> Node::GetParent() const
{
    return m_parent.lock();
}
