#pragma once
#include <memory>
#include <EditorWindow.hpp>
#include <EditorContext.hpp>
#include <Engine.hpp>

namespace Droplet::Editor::Scene
{
	/// @brief Represents the game view window in the editor, which displays the main camera view.
	class GameViewWindow : public EditorWindow
	{
	public:
		/// @brief Constructor with a pointer to the editor context
		/// @param p_editorContext is a pointer that holds information about scene, camera, interactions and renderer
		GameViewWindow(EditorContext *p_editorContext) :
			m_editorContext(p_editorContext) {}

		~GameViewWindow() = default;

	protected:

		/// @brief Initializes the game view window and sets its name.
		void InitImpl() override;

		void RenderToolbar() override;

		/// @brief Renders the main camera view and handles user interactions within the game view window.
		void RenderImpl() override;

	private:
		/// @brief Pointer to a variable that is shared across several scenes
		EditorContext *m_editorContext = nullptr;

		bool m_hoveringSGameView = false;
		bool m_hoveringPlayButton = false;
	};
}