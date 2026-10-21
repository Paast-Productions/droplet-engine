#include "Node.hpp"
#include "Scene.hpp"
#include "Component.hpp"
#include <SceneSystem/DefaultNodeBounds.hpp>
#include <tracy/public/tracy/Tracy.hpp>
#include <stdexcept>
#include <glm/gtc/matrix_transform.hpp>


using namespace Droplet::Scene;

Node::Node(const std::shared_ptr<Scene> &p_scene, const std::string &p_name) // TODO: Make weak pointer
	: m_name(p_name), m_transform(this), m_scene(p_scene), m_id(s_nextID.fetch_add(1, std::memory_order_relaxed))
{
    if (!m_scene.lock())
    {
        throw std::invalid_argument("Cannot create Node: Scene is null.");
    }

    m_bounds = std::make_shared<DefaultNodeBounds>(glm::vec3(0.0f), glm::vec3(0.5f));
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
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

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

void Node::GetRenderData(std::vector<Graphics::RenderData> &p_renderData) 
{
    if (!m_active)
    {
        return;
    }

    Graphics::RenderData rData
    {
        .transform = m_transform.GetMatrix(Transform::Space::World)
    };
    
    for (const auto &component : m_components)
    {
        // TODO: Skip component if it's inactive
        component->GetRenderData(rData);
    }

    p_renderData.push_back(rData);
    
    for (const std::shared_ptr<Node>& child : m_children)
    {
        child->GetRenderData(p_renderData);
    }

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

std::shared_ptr<Node> Node::AddChild(std::shared_ptr<Node> p_child) // TODO: Make weak pointer
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

void Node::RemoveChild(const std::shared_ptr<Node> &p_child) // TODO: Make weak pointer
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

void Droplet::Scene::Node::AttachComponent(std::unique_ptr<Component> &p_component)
{
    if (!p_component)
    {
        throw std::invalid_argument("Cannot add a null component");
    }

    if (m_nextComponentID == 0)
    {
        throw std::runtime_error("Invalid next component ID");
    }
    std::shared_ptr<Droplet::Scene::Component> shared = std::move(p_component);
    AttachComponent(shared, m_nextComponentID);
}


void Droplet::Scene::Node::AttachComponent(std::shared_ptr<Component> &p_component, std::uint64_t p_componentID)
{
    ZoneScoped;

    if (!p_component)
    {
        throw std::invalid_argument("Cannot add a null component");
    }

    if (p_componentID == 0)
    {
        throw std::invalid_argument(
            "Component ID 0 is reserved");
    }

    for (const std::shared_ptr<Component> &itComponent : m_components)
    {
        if (itComponent->GetID() == p_componentID)
        {
            throw std::runtime_error("Duplicate component ID on node");
        }
    }

    if (p_component->GetID() != 0)
    {
        throw std::overflow_error("No component IDs remain for this node");
    }

    p_component->m_id = p_componentID;
    p_component->SetOwner(shared_from_this());
    m_components.push_back((p_component));
    

    if (p_component->HasBoundsOverride())
    {
        if (m_boundsOverride.expired())
        {
            m_boundsOverride = p_component;
        }

        
    }

    

    if (p_componentID >= m_nextComponentID)
    {
        m_nextComponentID = p_componentID + 1;
    }

    if (m_started)
    {
        p_component->Start();
    }
}

std::shared_ptr<Node> Node::GetParent() const // TODO: Make weak pointer
{
    return m_parent.lock();
}

const std::vector<std::shared_ptr<Node>> &Node::GetChildren() const // TODO: Make weak pointer
{
    return m_children;
}

std::shared_ptr<Scene> Node::GetScene() const // TODO: Make weak pointer
{
    return m_scene.lock();
}

const std::string &Node::GetName() const
{
    return m_name;
}

void Droplet::Scene::Node::SetName(const std::string &p_name)
{
    m_name = p_name;
}

void Node::AddComponent(std::unique_ptr<Component> &p_component)
{
    if (!p_component)
    {
        throw std::invalid_argument("Cannot add nullptr Component.");
	}

    p_component->SetOwner(shared_from_this());

    m_components.push_back(std::move(p_component));
	std::shared_ptr<Component> addedComponent = m_components.back();

    // Check if the component being added provides bounds override.
    if (addedComponent->HasBoundsOverride())
    {
        if (m_boundsOverride.expired())
        {
		    m_boundsOverride = addedComponent;
        }
    }

    if (m_started)
    {
        addedComponent->Start();
	}
}

std::vector<std::shared_ptr<Component>> Node::GetAllComponents() const // TODO: Make weak pointer
{
    return m_components;
}

void Node::RemoveComponent(const Component *p_component) // TODO: Make weak pointer
{
    if (!p_component)
    {
        throw std::invalid_argument("Cannot remove nullptr Component.");
    }

    int index = 0;
    for (index = 0; index < m_components.size(); ++index)
    {
        if (m_components[index].get() == p_component)
        {
            break;
        }
    }

    if (index == m_components.size())
    {
        throw std::runtime_error("Cannot remove Component: Component is not attached to Node '" + m_name + "'.");
    }

    // Check if the component being removed provides the current bounds override.
    const bool isBoundsOverride = (p_component == m_boundsOverride.lock().get());

    m_components.erase(m_components.begin() + index);

    // If the removed component was the bounds override, find another one.
    if (isBoundsOverride)
    {
        m_boundsOverride.reset();

        for (const auto &component : m_components)
        {
            if (component->HasBoundsOverride())
            {
                m_boundsOverride = component;
                break;
            }
        }
    }
}

const NodeBounds *Node::GetBounds() const
{
    if (!m_boundsOverride.expired())
    {
		Component *overrideComponent = m_boundsOverride.lock().get();

		assert(overrideComponent != nullptr && "Bounds override component is null.");

		const NodeBounds *boundsOverride = overrideComponent->GetBounds();

		assert(boundsOverride != nullptr && "Bounds override is null.");

        return boundsOverride;
    }

	return m_bounds.get();
}

void Node::SetBounds(std::shared_ptr<NodeBounds> p_bounds)
{
    if (!p_bounds)
    {
        throw std::invalid_argument("Node bounds cannot be null.");
    }

    m_bounds = std::move(p_bounds);
}

bool Node::IsBoundsEnabled() const
{
    return m_boundsEnabled;
}

void Node::SetBoundsEnabled(bool p_enabled)
{
    m_boundsEnabled = p_enabled;
}

void Node::SetScene(std::shared_ptr<Scene> p_scene) // TODO: Make weak pointer
{
    m_scene = p_scene;

    for (const auto &child : m_children)
    {
        child->SetScene(p_scene);
    }
}

bool Node::IsDescendantOf(const std::shared_ptr<Node> &p_node) const // TODO: Make weak pointer
{
    auto parent = m_parent.lock();

    while (parent)
    {
        if (parent == p_node)
        {
            return true;
        }

        parent = parent->GetParent();
    }

    return false;
}

bool Node::IsRenderable() const
{
    for (const auto &component : m_components)
    {
        if (component && component.get()->m_renderable)
        {
            return true;
        }
    }
    return false;
}