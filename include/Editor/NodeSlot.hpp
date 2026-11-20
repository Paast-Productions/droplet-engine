#pragma once
#ifndef _GAME
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <ImGui/imgui.h>

namespace Droplet::Scene
{
    class Node;
}

namespace Droplet::Editor
{
	/// @brief UI for dragging and dropping a node onto a slot. The slot will display the node's name if it is valid, otherwise it will display the label.
    /// @param p_label ImGui label for the UI
	/// @param p_node Reference to a weak pointer to the node that will be set when a valid node is dropped onto the slot
	/// @param p_buttonSize Size of the button for the slot. If (0, 0), the button will be sized automatically.
    /// @param p_acceptCallback Function to check if a node can be accepted by the slot
    /// @param p_onClickCallback Function to be called when the slot is clicked
    /// @param p_noAcceptMsg Message to display when a node cannot be accepted
	/// @return true if a valid node was dropped onto the slot, false otherwise
    bool NodeSlot(
        const char *p_label,
        std::weak_ptr<Droplet::Scene::Node> &p_node,
		ImVec2 p_buttonSize = ImVec2(0, 0),
        std::optional<std::function<bool(Droplet::Scene::Node*)>> p_acceptCallback = std::nullopt,
        std::optional<std::function<void()>> p_onClickCallback = std::nullopt,
        const char *p_noAcceptMsg = "Cannot accept this node");

	/// @brief UI for dragging and dropping multiple nodes onto a slot. The slot will display the number of nodes if they are valid, otherwise it will display the label.
	/// @param p_label ImGui label for the UI
	/// @param p_nodes Reference to a vector of weak pointers to the nodes that will be set when valid nodes are dropped onto the slot
	/// @param p_maxNodes Maximum number of nodes that can be accepted by the slot. If -1, there is no limit.
	/// @param p_acceptCallback Function to check if a node can be accepted by the slot
	/// @param p_onClickCallback Function to be called when a slot is clicked
	/// @param p_noAcceptMsg Message to display when a node cannot be accepted
	/// @return true if a valid node was dropped onto the slot, false otherwise
	bool NodeListSlot(
		const char *p_label,
		std::vector<std::weak_ptr<Droplet::Scene::Node>> &p_nodes,
		int p_maxNodes = -1,
        std::optional<std::function<bool(Droplet::Scene::Node *)>> p_acceptCallback = std::nullopt,
        std::optional<std::function<void(int)>> p_onClickCallback = std::nullopt,
		const char *p_noAcceptMsg = "Cannot accept this node"
	);
}
#endif
