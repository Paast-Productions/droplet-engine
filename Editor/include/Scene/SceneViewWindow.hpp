#pragma once
#include <vector>
#include <memory>
#include <EditorWindow.hpp>
#include <EditorCamera.hpp>
#include <InteractionState.hpp>
#include <Engine.hpp>
#include <ImGuizmo/ImGuizmo.h>

namespace Droplet::Editor::Scene
{
	/// @brief Represents the scene view window in the editor, which displays the camera view and handles editor camera interactions.
	class SceneViewWindow : public EditorWindow
	{
	public:
		SceneViewWindow(std::shared_ptr<Droplet::Engine> p_engine, std::shared_ptr<InteractionState> p_interactionState) :
			EditorWindow(p_engine), m_interactionState(p_interactionState), m_editorCamera(p_interactionState) {}
		~SceneViewWindow() = default;

	protected:

		/// @brief Initializes the scene view window and sets its name.
		void InitImpl() override { SetName("Scene View"); ToggleToolbar(true); }

		void RenderToolbar() override;

		/// @brief Renders the editor camera view and handles user interactions within the scene view window.
		void RenderImpl() override;

	private:
		EditorCamera m_editorCamera;
		std::shared_ptr<InteractionState> m_interactionState;

		// ImGuizmo
		ImGuizmo::MODE m_manipulateSpace = ImGuizmo::LOCAL;

		void RenderImGuizmo(ImVec2 p_sceneViewMin, ImVec2 p_sceneViewMax);
	};
}