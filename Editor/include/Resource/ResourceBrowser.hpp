#pragma once
#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <string>
#include <Engine/Engine.hpp>
#include <core/FileExplorer.hpp>

namespace Droplet::Editor::Resource
{
	class ResourceBrowser : public EditorWindow
	{
	public:
		ResourceBrowser(std::shared_ptr<Engine> p_instance, FileExplorer &p_fileExplorer) 
	        : m_instance(p_instance), m_fileExplorer(p_fileExplorer) {}
	     
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
	    std::filesystem::path m_currentBrowserDirectory;
		GUID selectedResourceGUID = C_INVALID_GUID; /// The GUID of the currently selected resource in the resource browser.
	    
	    FileExplorer &m_fileExplorer;
	    std::queue<std::vector<MetaEntry>> m_registerPopupQueue;
	    
		/// @brief Engine instance used to access engine systems.
		std::shared_ptr<Engine> m_instance;
	};
}