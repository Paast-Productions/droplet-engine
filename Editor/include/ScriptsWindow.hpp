#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"

#include <filesystem>

namespace Droplet::Editor
{
	/// @brief Editor window for browsing and opening Lua scripts.
	/// ScriptsWindow displays scripts and directories in a tree structure,
	/// allowing developers to navigate the script directory and select
	/// scripts to open in the editor.
	class ScriptsWindow : public EditorWindow
	{
	public:
		/// @brief Constructs the scripts window.
		/// @param p_interactionState Shared editor interaction state.
		ScriptsWindow();
		/// @brief Destroys the scripts window.
		~ScriptsWindow() = default;

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
		/// @brief Represents a file or directory in the script tree.
		/// Directory nodes can contain child nodes, allowing the tree to
		/// represent nested directories and their contents.
		struct ScriptTreeNode
		{
			/// @brief Name of the file or directory.
			std::string name;

			/// @brief Path relative to the script root directory.
			std::filesystem::path relativePath;

			/// @brief Indicates whether this node represents a directory.
			bool isDirectory = false;

			/// @brief Child files and directories contained in this directory.
			std::vector<ScriptTreeNode> children;
		};

		/// @brief Recursively renders a node in the script tree.
		/// @param node Script tree node to render.
		void RenderTreeHelper(const ScriptTreeNode &node);

		/// @brief Refreshes the script tree from the script directory.
		/// Updates the tree to reflect the current files and directories.
		/// Any errors encountered during the refresh can be stored for display.
		void RefreshScripts();

		void RenderAddScript();

		void DeleteSelectedScript();

		/// @brief Opens the selected script.
		/// @param p_relativePath Path to the script relative to the script root.
		void OpenScript(const std::filesystem::path &p_relativePath);

		/// @brief Root directory containing the scripts.
		std::filesystem::path m_scriptPath;

		/// @brief Hierarchical representation of the script directory.
		std::vector<ScriptTreeNode> m_scriptTree;

		/// @brief Name or path of the currently selected script.
		std::string m_selectedScript;

		/// @brief Error message produced while refreshing the script tree.
		std::string m_refreshError;

		/// @brief Error message produced while opening a script.
		std::string m_openError;
	};
}