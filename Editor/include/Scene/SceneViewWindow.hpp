#pragma once
#include <vector>
#include <memory>
#include <EditorWindow.hpp>
#include <EditorCamera.hpp>
#include <InteractionState.hpp>

namespace Droplet::Editor::Scene
{
	class SceneViewWindow : public EditorWindow
	{
	public:
		SceneViewWindow(std::shared_ptr<InteractionState> p_interactionState) : m_interactionState(p_interactionState), m_editorCamera(p_interactionState) {}
		~SceneViewWindow() = default;

	protected:

		/// @brief Renders the editor camera view and handles user interactions within the scene view window.
		void RenderImpl() override;

	private:
		EditorCamera m_editorCamera;
		std::shared_ptr<InteractionState> m_interactionState;
	};
}