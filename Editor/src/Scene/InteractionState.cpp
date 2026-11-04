#include "InteractionState.hpp"

#include <algorithm>

using namespace Droplet::Editor;
using namespace Droplet::Scene;

void InteractionState::SelectNode(const std::weak_ptr<Node> &p_node, bool p_clearFirst)
{
    if (p_node.expired() || IsNodeSelected(p_node))
    {
        return;
    }

    if (p_clearFirst)
    {
        ClearNodeSelection();
    }

    m_selectedNodes.push_back(p_node);
}

void InteractionState::DeselectNode(const std::weak_ptr<Node> &p_node)
{
    if (p_node.expired())
    {
        return;
    }

    m_selectedNodes.erase(std::remove_if(
        m_selectedNodes.begin(), 
        m_selectedNodes.end(),
        [&p_node](const std::weak_ptr<Node> &p_selectedNode)
        {
            const auto node = p_selectedNode.lock();

            return !node || node == p_node.lock();
        }),
        m_selectedNodes.end());
}

void InteractionState::ClearNodeSelection()
{
    m_selectedNodes.clear();
}

bool InteractionState::IsNodeSelected(const std::weak_ptr<Node> &p_node) const
{
    if (p_node.expired())
    {
        return false;
    }

    for (const auto &selectedNode : m_selectedNodes)
    {
        const auto node = selectedNode.lock();

        if (node == p_node.lock())
        {
            return true;
        }
    }

    return false;
}

std::vector<std::weak_ptr<Node>> InteractionState::GetSelectedNodes() const
{
    std::vector<std::weak_ptr<Node>> selectedNodes;

    for (const auto &selectedNode : m_selectedNodes)
    {
        if (const auto node = selectedNode.lock())
        {
            selectedNodes.push_back(node);
        }
    }

    return selectedNodes;
}

std::vector<std::weak_ptr<Node>> InteractionState::GetSelectedRootNodes() const
{
    std::vector<std::weak_ptr<Node>> result;

    for (const auto &weakNode : m_selectedNodes)
    {
        auto node = weakNode.lock();
        if (!node)
        {
            continue;
        }

        bool ancestorSelected = false;

        for (auto parent = node->GetParent().lock();
            parent;
            parent = parent->GetParent().lock())
        {
            for (const auto &weakSelectedNode : m_selectedNodes)
            {
                auto selectedNode = weakSelectedNode.lock();

                if (selectedNode && selectedNode == parent)
                {
                    ancestorSelected = true;
                    break;
                }
            }

            if (ancestorSelected)
            {
                break;
            }
        }

        if (!ancestorSelected)
        {
            result.push_back(node);
        }
    }

    return result;
}

std::vector<std::weak_ptr<Node>> InteractionState::GetSelectedNodesAndChildren() const
{
	std::vector<std::weak_ptr<Node>> result;

	for (const auto &selectedNode : GetSelectedRootNodes())
	{
		if (const auto node = selectedNode.lock())
		{
			if (!node)
			{
				continue;
			}

			// Skip if the node is already in the result (to avoid duplicates)
			if (std::find_if(result.begin(), result.end(), [&](const std::weak_ptr<Node> &n) { return n.lock() == node; }) != result.end())
			{
                continue;
			}

			result.push_back(node);

			// Add all children of the selected node
			std::function<void(const std::shared_ptr<Node> &)> addChildren = [&](const std::shared_ptr<Node> &parent) {
				for (const auto &child : parent->GetChildren())
				{
					if (!child)
					{
						continue;
					}

					// Add the child if it's not already in the result
					if (std::find_if(result.begin(), result.end(), [&](const std::weak_ptr<Node> &n) { return n.lock() == child; }) != result.end())
					{
                        continue;
					}

					result.push_back(child);
                    addChildren(child);
				}
			};

			addChildren(node);
		}
	}

	return result;
}
