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
		BehaviourInspector(EditorContext *editorContext) : EditorWindow(editorContext) { }

	protected:
		void InitImpl() override;
		void RenderToolbar() override;
		void RenderImpl() override;
	};
}