#pragma once
#include "EditorWindow.hpp"

#include <vector>
#include <filesystem>
namespace Droplet::Editor
{

	class SavedScenesWindow : public EditorWindow
	{
	public:
		SavedScenesWindow();
		~SavedScenesWindow() = default;


	protected:
		void InitImpl() override;

		void RenderImpl() override;

		void CloseImpl() override;

		void RenderToolbar() override;

		

	private:

		void RefreshScannedFiles(std::vector < std::string> &p_sceneFiles);

		std::vector<std::string> m_sceneFiles;
	};
}