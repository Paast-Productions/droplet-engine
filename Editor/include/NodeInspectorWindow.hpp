#pragma once
#include <EditorWindow.hpp>
#include <SceneSystem/Node.hpp>
#include "InteractionState.hpp"
#include <EditorContext.hpp>

namespace Droplet::Editor
{
	/// @brief A class that inherits from EditorWindow.
	/// @details This class is supposed to show components from a selected node 
	/// and expose the data to the developer so that they can be edited.
	class NodeInspectorWindow : public EditorWindow
	{
	public:
		NodeInspectorWindow(EditorContext *p_editorContext);
		~NodeInspectorWindow() = default;

		/// @brief Sets the node a user sends to be the current node that will be displayed
		/// @param p_nodeToInspect The node that user wants to display
		void SetCurrentNode(std::shared_ptr<Droplet::Scene::Node> p_nodeToInspect); // TODO: Make weak pointer

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;

	private:
		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState; // TODO: Make weak pointer
		
		EditorContext *m_editorContext = nullptr;

		/// @brief The node that is currently displayed.
		std::shared_ptr<Droplet::Scene::Node> m_currentNode; // TODO: Make weak pointer

		/// @brief A variable to remember if the scale should be locked
		bool m_lockScale = true;
	};
}
