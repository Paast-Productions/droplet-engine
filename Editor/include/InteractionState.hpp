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

		// TODO: Add methods for managing the interaction state, such as selecting nodes, deselecting nodes, and querying the current selection.
		
		void SelectNode(const std::shared_ptr<Droplet::Scene::Node> &p_node);

		void clearSelection();
	private:

		std::vector<std::weak_ptr<Droplet::Scene::Node>> m_selectedNodes{};
	};
}