#pragma once
#include "EditorWindow.hpp"

#include <vector>
#include <filesystem>
namespace Droplet::Editor
{
	enum class SavedDataType
	{
		Prefab = 0,
		Scene = 1
	};

	struct SceneDragDropData
	{
		char path[256];
		char name[128];
	};

	class SavedDataWindow : public EditorWindow
	{
	public:
		SavedDataWindow();
		~SavedDataWindow() = default;


	protected:
		/// @brief Initilize the editing window
		void InitImpl() override;
		/// @brief Renders the window
		void RenderImpl() override;
		/// @brief Use this for special closing conditions
		void CloseImpl() override;
		/// @brief Used to render the tool bar
		void RenderToolbar() override;
	private:

		struct SceneTreeNode
		{
			std::string name;
			bool isDirectory = false;
			std::filesystem::path relativePath;
			std::vector<SceneTreeNode> children;
		};
		/// @brief Helps to render the file hierarchy
		/// @param node Node in the hierarchy
		/// @param type The type of file, right now either prefab or scene
		void RenderTreeHelper(const SceneTreeNode &node, SavedDataType type);

		/// @brief Deletes the selected file and makes a popup window appear
		void DeleteSelectedFile();

		/// @brief Opens the json file to the standard application, only works on windows
		/// @param p_path the file to open
		void OpenJson(const std::filesystem::path &p_path);

		/// @brief Searches through the directories to find all the json files, calls when being asked to refresh or when deleting a file
		/// @param p_sceneFiles the files
		void RefreshScannedFiles(std::vector < std::string> &p_sceneFiles);

		/// @brief Remember if we want to open delete window
		bool m_openDelete = false;

		std::vector<std::string> m_sceneFiles;
		/// @brief The scene tree hierachy
		std::vector<SceneTreeNode> m_sceneTree;
		/// @brief the prefab tree
		std::vector<SceneTreeNode> m_prefabTree;

		std::string m_selectedJsonScene;
		/// @brief Directory containing the scenes
		std::filesystem::path m_scenePath;
		/// @brief Directory containing the prefabs
		std::filesystem::path m_prefabPath;

		bool m_needsRefresh = false;
	};
}