#pragma once

#include "EditorWindow.hpp"
#include <EditorContext.hpp>

namespace Droplet::Editor
{
	/// @brief Editor window for browsing and opening Lua scripts.
	/// ScriptsWindow displays scripts and directories in a tree structure,
	/// allowing developers to navigate the script directory and select
	/// scripts to open in the editor.
	class BehaviourInspector : public EditorWindow
	{
	public:
		/// @brief Constructs the behaviour inspector.
		/// @param p_interactionState Shared editor interaction state.
		BehaviourInspector(EditorContext *editorContext);
		/// @brief Destroys the behaviour inspector.
		~BehaviourInspector() = default;

	protected:
		/// @brief Initializes the scripts window.
		/// Called by the base EditorWindow when the window is initialized.
		void InitImpl() override;

		/// @brief Closes the scripts window.
		/// Called by the base EditorWindow when the window is closed.
		void CloseImpl() override;

		/// @brief Renders the scripts window's main content.
		/// Displays the script directory tree and handles script selection.
		void RenderImpl() override;

		/// @brief Renders the scripts window's toolbar.
		/// Displays toolbar controls associated with the scripts window.
		void RenderToolbar() override;

	private:
		Droplet::Editor::EditorContext *m_editorContext;
	};
}