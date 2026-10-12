#pragma once
#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <string>

namespace Droplet::Editor::Resource
{
	class ResourceBrowser : public EditorWindow
	{
	public:

	protected:

		/// @brief Initializes the resource browser window and sets its name.
		void InitImpl() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
		GUID selectedResourceGUID = C_INVALID_GUID; /// The GUID of the currently selected resource in the resource browser.
	};
}