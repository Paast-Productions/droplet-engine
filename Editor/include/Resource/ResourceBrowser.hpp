#pragma once

#include <string>

#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <EditorContext.hpp>
#include <Common/RenderDesc.hpp>

namespace Droplet::Editor::Resource
{
    struct BrowserNode
    {
        std::string name;
        bool isDirectory = false;
        
        std::map<std::string, BrowserNode> children; // If it's a directory
        std::vector<const MetaEntry*> resources;     // If it's an asset file
    };

	struct PreviewCamera
	{
		Common::Camera camera{};

		Common::RenderTargetDesc target {};
		glm::ivec2 resolution{ 256, 256 };

		glm::vec3 pivot{}; // The point the camera orbits around and zooms towards
		float pivotRadius = 10.0f; // The distance from the camera to the pivot
		float fitRadius = 10.0f; // The distance from the pivot at which the mesh bounds fill the view

		StaticResourceHandle<Texture2DResource> texHandle {};
		StaticResourceHandle<ShaderResource> vsHandle {};
		StaticResourceHandle<ShaderResource> fsHandle {};
	};
    
	class ResourceBrowser : public EditorWindow
	{
	public:
	    ResourceBrowser(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {}
		~ResourceBrowser();
	     
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		void RenderToolbar() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
        /// @brief Builds a virtual directory hierarchy from a registry list.
        /// @param p_resources The registry of resources.
        void BuildBrowserTree(const std::vector<const MetaEntry*> &p_resources);

		/// @brief Draws the resource list in the left pane of the resource browser window.
		void DrawResourceList(const std::vector<const MetaEntry *> &p_resources, BrowserNode *p_currentNode);

		/// @brief Draws the resource details in the right pane of the resource browser window.
		void DrawResourceInspector(const MetaEntry &p_entry);

		void DrawResourcePreview(const MetaEntry &p_entry);

		/// @brief Frames the mesh preview camera on the mesh bounds, with the pivot at the bounds center.
		/// @param p_mesh The mesh being previewed. Left unchanged if its BVH has not been generated.
		void ResetMeshPreviewCamera(const MeshResource &p_mesh);

		/// @brief Applies mouse orbit, pan and zoom to the mesh preview camera. LMB orbits, MMB pans the pivot and the wheel zooms.
		/// @param p_imageSize The size of the displayed preview image in pixels.
		void ProcessMeshPreviewInput(const ImVec2 &p_imageSize);

		/// @brief Hides and locks the mouse cursor over the mesh preview, or restores it. Uses the last drawn item as the lock area.
		void SetPreviewCursorLocked(bool p_locked);

		/// @brief Moves the mesh preview camera so it looks at its pivot from its current pivot radius.
		void ApplyMeshPreviewPivot();
	    
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
        std::string ResourceTypeToString(StaticResourceType p_type);
	    
	    BrowserNode m_rootBrowserNode {}; // The root node in the resource browser hierarchy
	    std::filesystem::path m_currentBrowserPath; // The current directory of the resource browser
	    
		GUID m_selectedResourceGuid = C_INVALID_GUID; // The GUID of the currently selected resource in the resource browser.
	    MetaEntry m_inspectedEntry {}; // The current entry being inspected

		PreviewCamera m_previewCamera {}; // The camera used for rendering previews of resources in the resource browser
		StaticResourceHandle<IStaticResource> m_previewResourceHandle {}; // The handle of the resource currently being inspected
		bool m_isPreviewResourceLoaded = false; // True if the preview resource has been loaded and is ready to be drawn
		bool m_resetPreviewCamera = false; // True if the mesh preview camera should be framed on the mesh once it has loaded
		bool m_previewCursorLocked = false; // True while the cursor is hidden and locked by a mesh preview drag
		ImVec2 m_previewCursorRestorePos {}; // Window-relative cursor position captured when the preview lock began
		
	    bool m_showUnsupportedPopup = false; // True if a popup showing unsupported assets should be rendered
	    std::string m_unsupportedFilesList {}; // A list of unsupported assets in the latest registering attempt
	    std::queue<std::vector<MetaEntry>> m_registerPopupQueue {}; // Queued popups upon successful resource registration
		
		std::unordered_map<std::uint64_t, std::uint32_t> m_imGuiTexMap {};

		std::string m_searchFilter = "";
	};
}
