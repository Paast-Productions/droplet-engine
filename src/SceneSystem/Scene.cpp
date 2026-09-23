#include "Scene.hpp"
#include "Node.hpp"
#include <utility>
#include <stdexcept>

using namespace Droplet::Scene;

Scene::Scene(const std::string &p_name)
    : m_name((p_name))
{
}

void Scene::Update(float p_deltaTime)
{
    if (!m_loaded || !m_active)
    {
        return;
    }

    for (const auto &root : m_roots)
    {
        root->Update(p_deltaTime);
    }
}


void Scene::Load()
{
    if (m_loaded)
    {
        throw std::runtime_error(
            "Cannot load Scene '" + m_name + "': Scene is already loaded.");
    }

    // TODO: Implement scene loading

    m_loaded = true;
}

void Scene::Unload()
{
    if (!m_loaded)
    {
        throw std::runtime_error(
            "Cannot unload Scene '" + m_name + "': Scene is not loaded.");
    }

    m_active = false;
    m_loaded = false;
}

void Scene::Render()
{
    if (!m_loaded || !m_active)
    {
        return;
    }

    for (const auto &root : m_roots)
    {
        root->Render();
    }
}

std::shared_ptr<Node> Scene::AddNode(const std::string &p_name)
{
    auto node = std::make_shared<Node>(shared_from_this(), p_name);

    m_roots.push_back(node);

    if (m_active)
    {
        node->Start();
    }

    return node;
}

void Scene::SetRoot(const std::shared_ptr<Node> &p_node, bool p_makeRoot)
{
    if (!p_makeRoot)
    {
        RemoveRoot(p_node);
        return;
    }

    // Ensure node is not already a root node
    if (std::find(m_roots.begin(), m_roots.end(), p_node) != m_roots.end())
    {
		throw std::runtime_error(
			"Cannot add Node '" + p_node->GetName() + "' as a root: Node is already a root of this Scene."
		);
    }

    // Ensure node has no parent
    if (p_node->GetParent())
	{
		throw std::runtime_error(
			"Cannot add Node '" + p_node->GetName() + "' as a root: Node has a parent."
		);
    }

    m_roots.push_back(p_node);
}
   
void Scene::RemoveRoot(const std::shared_ptr<Node> &p_node)
{
    if (!p_node)
    {
        throw std::invalid_argument("Cannot remove null Node from Scene.");
    }

    auto it = std::find(m_roots.begin(), m_roots.end(), p_node);

    if (it == m_roots.end())
    {
        throw std::runtime_error( "Cannot remove Node '" + p_node->GetName() + "': Node is not a root of this Scene.");
    }

    m_roots.erase(it);
}

const std::vector<std::shared_ptr<Node>> &Scene::GetRoots() const
{
    return m_roots;
}

void Node::SetScene(std::shared_ptr<Scene> p_scene)
{
    m_scene = p_scene;

    for (const auto &child : m_children)
    {
        child->SetScene(p_scene);
    }
}

std::shared_ptr<Scene> Node::GetScene() const
{
    return m_scene.lock();
}

const std::string &Scene::GetName() const
{
    return m_name;
}

bool Scene::IsLoaded() const
{
    return m_loaded;
}

bool Scene::IsActive() const
{
    return m_active;
}


void Scene::SetActive(bool p_active)
{
    if (m_active == p_active)
    {
        return;
    }

    if (p_active && !m_loaded)
    {
        throw std::runtime_error(
            "Cannot activate Scene '" + m_name + "': Scene is not loaded.");
    }

    m_active = p_active;

    if (m_active)
    {
        for (const auto &root : m_roots)
        {
            root->Start();
        }
    }
}