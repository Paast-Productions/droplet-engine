#pragma once

#include "EditorWindow.hpp"
#include "InteractionState.hpp"

#include <filesystem>

namespace Droplet::Editor
{
	/// @brief A class that inherits from EditorWindow.
	/// @details This class is supposed to show components from a selected node 
	/// and expose the data to the developer so that they can be edited.
	class ScriptWindow : public EditorWindow
	{
	public:
		ScriptWindow(std::shared_ptr<InteractionState> p_interactionState);
		~ScriptWindow() = default;

	protected:
		void InitImpl() override;
		void CloseImpl() override;
		void RenderImpl() override;
		void RenderToolbar() override;

	private:
		struct ScriptTreeNode
		{
			std::string name;
			std::filesystem::path relativePath;
			bool isDirectory = false;
			std::vector<ScriptTreeNode> children;
		};

		void RenderTreeHelper(const ScriptTreeNode& node);
		void RefreshScripts();

	private:

		/// @brief Shared editor interaction state used to track node selection.
		std::shared_ptr<InteractionState> m_interactionState;
		/// @brief Path for all the scripts

		std::filesystem::path m_scriptPath;
		std::vector<ScriptTreeNode> m_scriptTree;

		std::string m_selectedScript;
		std::string m_refreshError;
	};
}
