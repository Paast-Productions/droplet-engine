#pragma once
#include <SceneSystem/Node.hpp>
namespace Droplet::Editor
{

	class EditorContext;
	/// @brief Editor shortcuts is a quality of life class that is responsible for making the editor more efficient to use.
	/// Introducing keybinds that affect what you change in certain windows.
	class EditorShortcuts
	{
	public:
		
		static EditorShortcuts &GetInstance()
		{
			static EditorShortcuts instance;
			return instance;
		}
		~EditorShortcuts() = default;

		/// @brief Processes key shortcuts for the scene view window.
		void ProcessSceneViewKeyShortcuts(EditorContext &p_editorContext);
	private:
		EditorShortcuts() = default;

		EditorShortcuts(const EditorShortcuts &) = delete;
		EditorShortcuts &operator=(const EditorShortcuts &) = delete;
		/// @brief Copies all the selected nodes
		/// @param p_editorContext 
		void CopySelectedNodes(EditorContext &p_editorContext);
		/// @brief Pastes all the nodes that is in the clipboard
		/// @param p_editorContext 
		void PasteNodes(EditorContext &p_editorContext);
	};
}
