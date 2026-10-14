#pragma once
#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <string>
#include <Engine/Engine.hpp>
#include <core/FileExplorer.hpp>

namespace Droplet::Editor::Resource
{
    struct BrowserNode
    {
        std::string name;
        bool isDirectory = false;
        
        std::map<std::string, BrowserNode> children; // If it's a directory
        std::vector<const MetaEntry*> resources;     // If it's an asset file
    };
    
	class ResourceBrowser : public EditorWindow
	{
	public:
	    ResourceBrowser(std::shared_ptr<Engine> p_engine, FileExplorer &p_fileExplorer) 
	        : EditorWindow(p_engine), m_engine(p_engine), m_fileExplorer(p_fileExplorer)  {}
	     
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
        /// @brief Builds a virtual directory hierarchy from a registry list.
        /// @param p_resources The registry of resources.
        void BuildBrowserTree(const std::vector<const MetaEntry*> &p_resources);
	    
	    void DrawBrowserNode(BrowserNode &p_node);
	    
        /// @brief Draws the meta settings for a resource.
        /// @param p_entry The meta entry to draw settings for.
        /// @return True if any of the settings were changed, otherwise false.
        bool DrawResourceSettings(MetaEntry &p_entry) const;

        /// @brief Converts a resource type enum to a string.
        /// @param p_type The resource type to convert.
        /// @return String representing the resource type.
        std::string ResourceTypeToString(ResourceType p_type);
	    
	    std::filesystem::path m_currentBrowserPath;
	    BrowserNode m_rootBrowserNode;
		GUID m_selectedResourceGuid = C_INVALID_GUID; /// The GUID of the currently selected resource in the resource browser.
	    
	    FileExplorer &m_fileExplorer;
	    std::queue<std::vector<MetaEntry>> m_registerPopupQueue;
	    
		/// @brief Engine instance used to access engine systems.
		std::shared_ptr<Engine> m_engine;
	};
}