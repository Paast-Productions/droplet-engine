#pragma once
#include <EditorWindow.hpp>
#include <SceneSystem/Node.hpp>
#include "InteractionState.hpp"

namespace Droplet::Editor
{
	/// @brief A class that inherits from EditorWindow.
	/// @details This class is supposed to show components from a selected node 
	/// and expose the data to the developer so that they can be edited.
	class NodeInspectorWindow : public EditorWindow
	{
	public:
		NodeInspectorWindow(std::shared_ptr<InteractionState> p_interactionState);
		~NodeInspectorWindow() = default;

		void SetCurrentNode(std::shared_ptr<Droplet::Scene::Node> p_nodeToInspect);

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;

	private:
		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState;

		/// @brief The node that is currently displayed.
		std::shared_ptr<Droplet::Scene::Node> m_currentNode;

		/// @brief A variable to remember if the scale should be locked
		bool m_lockScale = true;
	};
}
