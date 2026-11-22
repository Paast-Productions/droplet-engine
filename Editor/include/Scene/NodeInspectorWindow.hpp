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
		/// @brief Drag field that snaps its value to an increment when snapping is active.
		/// @details Keeps the unsnapped value between frames so small mouse deltas accumulate instead of being rounded away.
		/// @param p_label The label of the drag field, also used as its ID.
		/// @param p_value The value to edit, only written when the field changes.
		/// @param p_speed The drag speed passed to ImGui.
		/// @param p_snapIncrement The increment to snap to. Pass 0 to disable snapping.
		/// @param p_relative If true, snap relative to the value at the start of the drag. If false, snap to multiples of zero.
		/// @return True if the value changed.
		bool DragFloat3Snapped(const char *p_label, glm::vec3 &p_value, float p_speed, float p_snapIncrement, bool p_relative, int *p_changedAxis = nullptr);

		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState; // TODO: (SCENE) Make weak pointer

		/// @brief The node that is currently displayed.
		std::weak_ptr<Droplet::Scene::Node> m_currentNode;

		/// @brief A variable to remember if the scale should be locked
		bool m_lockScale = false;

		/// @brief Unsnapped value and drag-start value of the drag field currently being dragged.
		struct SnapDragState
		{
			ImGuiID id = 0;
			glm::vec3 value{};
			glm::vec3 start{};
		} m_snapDrag;
	};
}
