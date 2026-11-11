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
		NodeInspectorWindow(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {}
		~NodeInspectorWindow() = default;

		/// @brief Sets the node a user sends to be the current node that will be displayed
		/// @param p_nodeToInspect The node that user wants to display
		void SetCurrentNode(std::weak_ptr<Droplet::Scene::Node> p_nodeToInspect);

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;

	private:
		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState; // TODO: Make weak pointer

		/// @brief The node that is currently displayed.
		std::weak_ptr<Droplet::Scene::Node> m_currentNode;

		/// @brief A variable to remember if the scale should be locked
		bool m_lockScale = false;

		/// @brief Independent axis angles (degrees) shown in the rotation control.
		glm::vec3 m_displayedRotation = glm::vec3(0.0f);

		const Droplet::Scene::Node *m_rotationNode = nullptr;
		Droplet::Scene::Transform::Space m_rotationSpace = Droplet::Scene::Transform::Space::World;
		glm::quat m_lastDisplayedRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		bool m_hasDisplayedRotation = false;
	};
}
