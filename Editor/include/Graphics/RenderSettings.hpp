#pragma once

#include <string>
#include <ShaderOverride.hpp>
#include <EditorWindow.hpp>
#include <EditorContext.hpp>

namespace Droplet::Editor::Graphics
{
	class RenderSettings : public EditorWindow
	{
	public:
		RenderSettings(EditorContext *p_editorContext) : EditorWindow(p_editorContext) {}
		~RenderSettings();
	protected:

		/// @brief Initializes the resource browser window and sets its name.	
		void InitImpl() override;

		void RenderToolbar() override;

		/// @brief Renders the resource browser window and handles user interactions within the window.
		void RenderImpl() override;

	private:

		static float m_clearColor[3];

		std::string m_fragmentOptions[5] = { "None", "Basic", "Color", "Normal", "UV" };
		std::string m_vertexOptions[2] = { "None", "Basic"};
	};
}
