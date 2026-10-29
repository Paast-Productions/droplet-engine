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
		void InitImpl() override;

		void RenderImpl() override;

		void CloseImpl() override;

		void RenderToolbar() override;

		

	private:

		struct SceneTreeNode
		{
			std::string name;
			bool isDirectory = false;
			std::filesystem::path relativePath;
			std::vector<SceneTreeNode> children;
		};

		void RenderTreeHelper(const SceneTreeNode &node, SavedDataType type);

		void DeleteSelectedFile();

		void OpenJson(const std::filesystem::path &p_path);

		void RefreshScannedFiles(std::vector < std::string> &p_sceneFiles);

		bool m_openDelete = false;

		std::vector<std::string> m_sceneFiles;

		std::vector<SceneTreeNode> m_sceneTree;

		std::vector<SceneTreeNode> m_prefabTree;

		std::string m_selectedJsonScene;
		/// @brief Directory containing the scenes
		std::filesystem::path m_scenePath;

		std::filesystem::path m_prefabPath;

		bool m_needsRefresh = false;
	};
}