#include "Node.hpp"
#include "Scene.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Component.hpp"
#include <stdexcept>

using namespace Droplet::Scene;

Node::Node(std::shared_ptr<Scene> p_scene, const std::string &p_name)
	: m_scene(p_scene), m_name(p_name), m_transform(this)
{
    if (!m_scene.lock())
    {
        throw std::invalid_argument("Cannot create Node: Scene is null.");
    }
}

void Node::Start()
{
    auto scene = m_scene.lock();

	if (!scene || !scene->IsActive())
    {
        return;
    }

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

	// TODO: Find an appropriate place to update transform if dirty. Should be done as late in the frame as possible, but before rendering.

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

void Node::RenderUI()
{
    // TODO: implement node UI rendering wrapper logic

	// Recursively call RenderUI on components
    for (const auto &component : m_components)
    {
        component->RenderUI();
    }

    // TODO: implement node UI rendering wrapper logic
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
    if (!m_active)
    {
        return false;
    }

	// If the Node has a parent, it is only considered active if its parent is also active.
    if (auto parent = m_parent.lock())
    {
        return parent->IsActive();
    }

	return true;
}

bool Node::IsActiveSelf() const
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

	// Check for cycles in the hierarchy
    if (auto parent = GetParent())
    {
        while (parent)
        {
            if (parent == p_child)
            {
                throw std::runtime_error(
                    "Cannot add Node '" + p_child->GetName() + "' as a child of '" + m_name + "': "
                    "Adding a Node as a child of its own descendant would create a cycle."
                );
            }
            parent = parent->GetParent();
        }
    }

    if (p_child->GetScene() != m_scene.lock())
    {
		throw std::runtime_error(
            "Cannot add Node '" + p_child->GetName() + "' as a child of '" + m_name + "': "
            "Nodes belong to different Scenes."
        );
    }

    if (auto prevParent = p_child->GetParent())
    {
		prevParent->RemoveChild(p_child);
    }
    else
    {
		// Child was a root. Remove it from the scene's root nodes.
		m_scene.lock()->SetRoot(p_child, false);
    }

    p_child->m_parent = shared_from_this();

    m_children.push_back(p_child);

    // TODO: Consider if start should be called here or ex. in the Nodes constructor
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

	if (p_child->GetScene() != m_scene.lock())
	{
		throw std::runtime_error(
			"Cannot remove Node '" + p_child->GetName() + "' from '" + m_name + "': "
			"Nodes belong to different Scenes."
		);
	}

    auto it = std::find(m_children.begin(), m_children.end(), p_child);

    if (it == m_children.end())
    {
        throw std::runtime_error("Cannot remove Node '" + p_child->GetName() + "': Node is not a child of '" + m_name + "'.");
    }

    p_child->m_parent.reset();

    m_children.erase(it);

	// Add the removed child to the scene's root nodes
    m_scene.lock()->SetRoot(p_child, true);
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
