#include "Scene.hpp"
#include "Node.hpp"
#include "SceneSerializer.hpp"
#include "IoManager.hpp"

#include <utility>
#include <stdexcept>
#include <tracy/public/tracy/Tracy.hpp>
#include <iostream>
#include <EngineFlags.hpp>
#include <Time.hpp>
#include <SceneSystem/Components/PhysicsComponent.hpp>

using namespace Droplet::Scene;

Scene::Scene(const std::string &p_name)
    : m_name((p_name))
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

}

Scene::~Scene()
{
    for (std::shared_ptr<Node> &node : m_roots)
    {
        std::vector<std::shared_ptr<PhysicsComponent>> comps = node->GetComponents<PhysicsComponent>();
        for (std::shared_ptr<PhysicsComponent> comp : comps)
        {
            node->RemoveComponent(comp.get());
        }
    }
}

void Scene::Update()
{
	ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    if (!m_loaded || !m_active)
    {
        return;
    }

    if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipNodeLogic))
    {
        for (const auto &root : m_roots)
        {
            root->Update();
        }
    }

    if (EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::InEditor))
    {
        for (const auto &root : m_roots)
        {
            root->EditorUpdate();
        }
    }

    m_octree.Update();
}

void Droplet::Scene::Scene::LateUpdate()
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    if (!m_loaded || !m_active)
    {
        return;
    }

    if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipNodeLogic))
    {
        for (const auto &root : m_roots)
        {
            root->LateUpdate();
        }
    }

    for (const auto &root : m_roots)
    {
        root->GetTransform().ResetFrame();
    }
}

void Droplet::Scene::Scene::FixedUpdate()
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    if (!m_loaded || !m_active)
    {
        return;
    }

    if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipNodeLogic))
    {
        for (const auto &root : m_roots)
        {
            root->FixedUpdate();
        }
        
        if (m_physics != nullptr)
        {
            m_physics->FixedUpdate();
        }
    }
}

void Scene::Load()
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    if (m_loaded)
    {
        throw std::runtime_error(
            "Cannot load Scene '" + m_name + "': Scene is already loaded.");
    }

    // TODO: Implement scene loading
    m_octree.Initialize(glm::vec3(0.0f), glm::vec3(100.0f));

    m_loaded = true;
}

void Scene::Unload()
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    if (!m_loaded)
    {
        throw std::runtime_error(
            "Cannot unload Scene '" + m_name + "': Scene is not loaded.");
    }

    m_active = false;
    m_loaded = false;
}

void Scene::GetRenderData(std::vector<Graphics::RenderData> &p_renderData)
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    //TODO: Add rendering conditions
    if (!m_loaded /*|| !m_active*/)
    {
        return;
    }

    //TODO: Readd when editor user can toggle/change the flags in some way
    /*if (!EngineFlagsOwner::IsUpdateFlagSet(UpdateFlags::SkipAutoRender))
    {*/
        for (const auto &root : m_roots)
        {
            root->GetRenderData(p_renderData);
        }
    /*}*/
}

std::shared_ptr<Node> Scene::AddNode(const std::string &p_name) // TODO: Make weak pointer
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

    auto node = std::make_shared<Node>(shared_from_this(), p_name);

    m_roots.push_back(node);
    m_octree.AddNode(node);

    if (m_active)
    {
        node->Start();
    }

    return node;
}

void Scene::SetRoot(const std::shared_ptr<Node> &p_node, bool p_makeRoot) // TODO: Make weak pointer
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

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
   
void Scene::RemoveRoot(const std::shared_ptr<Node> &p_node) // TODO: Make weak pointer
{
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

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

const std::vector<std::shared_ptr<Node>> &Scene::GetRoots() const // TODO: Make weak pointer
{
    return m_roots;
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
    ZoneScoped;
    ZoneText(m_name.c_str(), m_name.size());

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

void Scene::PrintOctree()
{
    std::cout << m_octree.ToGraphviz();
}

void Scene::PrintOctreeTree()
{
    std::cout << m_octree.ToGraphvizTree();
}

std::weak_ptr<Node> Scene::RaycastNode(const Math::Ray &p_ray, Math::RayHit *p_hit, std::vector<std::weak_ptr<Node>> *p_ignoreNodes)
{
	return m_octree.GetNodeFromRaycast(p_ray, p_hit, p_ignoreNodes);
}

std::vector<std::weak_ptr<Node>> Scene::RaycastNodes(const Math::Ray &p_ray)
{
    return m_octree.GetNodesFromRaycast(p_ray);
}

std::vector<std::shared_ptr<Node>> Scene::GetNodesFullyInFrustum(const Math::Frustum &p_frustum)
{
    std::vector<std::shared_ptr<Node>> nodes;
    m_octree.GetNodesFullyInFrustum(p_frustum, nodes);
    return nodes;
}

void Droplet::Scene::Scene::InitializePhysics(const Physics::Initializer p_initializer)
{
    if (m_physics != nullptr)
    {
        return;
    }

    m_physics = std::make_unique<Physics>(p_initializer);
}

Droplet::Physics *Droplet::Scene::Scene::GetPhysics()
{
    return m_physics.get();
}

void Droplet::Scene::Scene::LoadNodeFromFile(const std::string& p_filepath)
{
    SceneSerializer serializer;
    nlohmann::json json = Droplet::Core::JsonIO::Read(p_filepath);

    serializer.DeserializeLonsesomeNode(json, *this);
}
