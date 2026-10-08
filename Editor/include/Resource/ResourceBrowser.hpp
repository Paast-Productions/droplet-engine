#pragma once
#include <EditorWindow.hpp>
#include <ResourceManager.hpp>
#include <string>
#include <Engine/Engine.hpp>

namespace Droplet::Editor::Resource
{
	class ResourceBrowser : public EditorWindow
	{
	public:
		ResourceBrowser(std::shared_ptr<Droplet::Engine> p_instance) : m_instance(p_instance) {}
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:
		GUID selectedResourceGUID = C_INVALID_GUID; /// The GUID of the currently selected resource in the resource browser.

		/// @brief Engine instance used to access engine systems.
		std::shared_ptr<Droplet::Engine> m_instance;
	};
}