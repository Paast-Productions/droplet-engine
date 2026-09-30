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
        void SelectNode(const std::shared_ptr<Scene::Node> &p_node);

        /// @brief Deselects a node.
        /// @param p_node The node to deselect.
        void DeselectNode(const std::shared_ptr<Scene::Node> &p_node);

        /// @brief Clears all selected nodes.
        void ClearNodeSelection();

        /// @brief Checks whether a node is selected.
        /// @param p_node The node to check.
        /// @return True if the node is selected.
        [[nodiscard]] bool IsNodeSelected(const std::shared_ptr<Scene::Node> &p_node) const;

        /// @brief Gets all currently selected nodes.
        /// @return The currently selected nodes.
        [[nodiscard]] std::vector<std::shared_ptr<Scene::Node>>GetSelectedNodes() const;

	private:

		std::vector<std::weak_ptr<Droplet::Scene::Node>> m_selectedNodes{};
	};
}