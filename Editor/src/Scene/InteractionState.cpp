#include "InteractionState.hpp"

#include <algorithm>

using namespace Droplet::Editor;
using namespace Droplet::Scene;

void InteractionState::SelectNode(const std::shared_ptr<Node> &p_node, bool p_clearFirst) // TODO: Make weak pointer
{
    if (!p_node || IsNodeSelected(p_node))
    {
        return;
    }

    if (p_clearFirst)
    {
        ClearNodeSelection();
    }

    m_selectedNodes.push_back(p_node);
}

void InteractionState::DeselectNode(const std::shared_ptr<Node> &p_node) // TODO: Make weak pointer
{
    if (!p_node)
    {
        return;
    }

    m_selectedNodes.erase(std::remove_if(
        m_selectedNodes.begin(), 
        m_selectedNodes.end(),
        [&p_node](const std::weak_ptr<Node> &p_selectedNode)
        {
            const auto node = p_selectedNode.lock();

            return !node || node == p_node;
        }),
        m_selectedNodes.end());
}

void InteractionState::ClearNodeSelection()
{
    m_selectedNodes.clear();
}

bool InteractionState::IsNodeSelected(const std::shared_ptr<Node> &p_node) const // TODO: Make weak pointer
{
    if (!p_node)
    {
        return false;
    }

    for (const auto &selectedNode : m_selectedNodes)
    {
        const auto node = selectedNode.lock();

        if (node == p_node)
        {
            return true;
        }
    }

    return false;
}

std::vector<std::shared_ptr<Node>> InteractionState::GetSelectedNodes() const // TODO: Make weak pointer
{
    std::vector<std::shared_ptr<Node>> selectedNodes; // TODO: Make weak pointer

    for (const auto &selectedNode : m_selectedNodes)
    {
        if (const auto node = selectedNode.lock())
        {
            selectedNodes.push_back(node);
        }
    }

    return selectedNodes;
}

std::vector<std::weak_ptr<Droplet::Scene::Node>> Droplet::Editor::InteractionState::GetSelectedRootNodes() const
{
    std::vector<std::weak_ptr<Node>> result;

    for (auto &node : m_selectedNodes)
    {
        const auto n = node.lock();
        if (!n)
        {
            continue;
        }

        bool ancestorSelected = false;
        for (auto parent = n->GetParent(); parent; parent = parent->GetParent())
        {
			bool found = false;
			for (const auto &selectedNode : m_selectedNodes)
			{
				const auto selected = selectedNode.lock();
				if (selected == parent)
				{
					found = true;
					break;
				}
			}

			if (found)
			{
				ancestorSelected = true;
				break;
			}
        }

        if (ancestorSelected)
        {
            continue;
        }

        result.push_back(node);
    }

    return result;
}
