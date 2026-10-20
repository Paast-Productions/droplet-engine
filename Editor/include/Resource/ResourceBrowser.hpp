#pragma once
#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <string>
#include <Engine/Engine.hpp>
#include <core/FileExplorer.hpp>
#include <EditorContext.hpp>

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
	    ResourceBrowser(EditorContext *p_editorContext);
	     
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
        /// @brief Builds a virtual directory hierarchy from a registry list.
        /// @param p_resources The registry of resources.
        void BuildBrowserTree(const std::vector<const MetaEntry*> &p_resources);
	    
        /// @brief Draws the meta settings for a resource.
        /// @param p_entry The meta entry to draw settings for.
        void DrawResourceSettings(MetaEntry &p_entry) const;

        /// @brief Opens a file explorer window and attempts to register all selected assets. Successfully registering
        /// an asset will queue a register popup that will prompt the user to modify the settings for the resources in
        /// that asset.
        void RegisterAsset();

        /// @brief Converts a resource type enum to a string.
        /// @param p_type The resource type to convert.
        /// @return String representing the resource type.
        std::string ResourceTypeToString(ResourceType p_type);

	    EditorContext *m_editorContext;
	    
	    BrowserNode m_rootBrowserNode; // The root node in the resource browser hierarchy
	    std::filesystem::path m_currentBrowserPath; // The current directory of the resource browser
	    
		GUID m_selectedResourceGuid = C_INVALID_GUID; // The GUID of the currently selected resource in the resource browser.
	    MetaEntry m_inspectedEntry; // The current entry being inspected
	    
	    bool m_showUnsupportedPopup = false; // True if a popup showing unsupported assets should be rendered
	    std::string m_unsupportedFilesList; // A list of unsupported assets in the latest registering attempt
	    std::queue<std::vector<MetaEntry>> m_registerPopupQueue; // Queued popups upon successful resource registration
	};
}
