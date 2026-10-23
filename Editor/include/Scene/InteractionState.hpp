#pragma once
#include <SceneSystem/Node.hpp>
#include <vector>
#include <memory>

namespace Droplet::Editor
{
	/// @brief Represents the state of user interactions within the editor, such as tracking selected nodes.
	class InteractionState
	{
	public:
		InteractionState() = default;
		~InteractionState() = default;

        /// @brief Selects a node.
        /// @param p_node The node to select.
        /// @param p_clearFirst Whether to clear the current selection before selecting the node.
        void SelectNode(const std::shared_ptr<Droplet::Scene::Node> &p_node, bool p_clearFirst = true); // TODO: Make weak pointer

        /// @brief Deselects a node.
        /// @param p_node The node to deselect.
        void DeselectNode(const std::shared_ptr<Droplet::Scene::Node> &p_node); // TODO: Make weak pointer

        /// @brief Clears all selected nodes.
        void ClearNodeSelection();

        /// @brief Checks whether a node is selected.
        /// @param p_node The node to check.
        /// @return True if the node is selected.
        [[nodiscard]] bool IsNodeSelected(const std::shared_ptr<Droplet::Scene::Node> &p_node) const; // TODO: Make weak pointer

        /// @brief Gets all currently selected nodes.
        /// @return The currently selected nodes.
        [[nodiscard]] std::vector<std::shared_ptr<Droplet::Scene::Node>> GetSelectedNodes() const; // TODO: Make weak pointer

	private:

		std::vector<std::weak_ptr<Droplet::Scene::Node>> m_selectedNodes{};
	};
}
