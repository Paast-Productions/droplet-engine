#include "InteractionState.hpp"

#include <algorithm>

using namespace Droplet::Editor;
using namespace Droplet::Scene;

void InteractionState::SelectNode(const std::shared_ptr<Node> &p_node)
{
    if (!p_node || IsNodeSelected(p_node))
    {
        return;
    }

    m_selectedNodes.push_back(p_node);
}

void InteractionState::DeselectNode(const std::shared_ptr<Node> &p_node)
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

bool InteractionState::IsNodeSelected(const std::shared_ptr<Node> &p_node) const
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

std::vector<std::shared_ptr<Node>>InteractionState::GetSelectedNodes() const
{
    std::vector<std::shared_ptr<Node>> selectedNodes;

    for (const auto &selectedNode : m_selectedNodes)
    {
        if (const auto node = selectedNode.lock())
        {
            selectedNodes.push_back(node);
        }
    }

    return selectedNodes;
}