#pragma once
#include <ImGui/imgui.h>
#include <string>

namespace Droplet::Editor
{
	/// @brief The base class for all editor windows.
	/// @details This class provides the basic functionality for all editor windows, including initialization and rendering. 
	/// Derived classes must implement the RenderImpl() method to define their specific rendering behavior.
	/// They may also override the InitImpl() and CloseImpl() methods to provide additional initialization and cleanup logic if needed.
	class EditorWindow
	{
	public:
		EditorWindow() = default;
		~EditorWindow() = default;

		/// @brief Initializes the editor window.
		void Init();

		/// @brief Closes the editor window.
		void Close();

		/// @brief Renders the editor window.
		void Render();

		/// @brief Toggles the visibility of the toolbar.
		/// @param visible True to show the toolbar, false to hide it.
		void ToggleToolbar(bool visible) { m_hasToolbar = visible; }
		
		/// @brief Checks if the editor window has a toolbar.
		/// @return True if the editor window has a toolbar, false otherwise.
		[[nodiscard]] bool HasToolbar() const { return m_hasToolbar; }

		/// @brief Gets the name of the editor window.
		/// @return The name of the editor window.
		[[nodiscard]] const std::string &GetName() const { return m_name; }
	
		/// @brief Gets the window flags for the editor window.
		/// @return The ImGui window flags for the editor window.
		[[nodiscard]] ImGuiWindowFlags GetWindowFlags() const { return m_windowFlags; }

	protected:

		/// @brief Initializes the editor window. Derived classes can override this method to provide additional initialization logic.
		virtual void InitImpl() { }

		/// @brief Closes the editor window. Derived classes can override this method to provide additional cleanup logic.
		virtual void CloseImpl() { }

		/// @brief Renders the editor window. Derived classes must implement this method to define their specific rendering behavior.
		virtual void RenderImpl() = 0;

		/// @brief Renders the toolbar for the editor window. Derived classes can override this method to provide custom toolbar rendering.
		virtual void RenderToolbar() { };

		/// @brief Sets the name of the editor window.
		/// @param name The name to set for the editor window.
		void SetName(const std::string &name) { m_name = name; }
		
		/// @brief Sets the window flags for the editor window.
		/// @param flags The ImGui window flags to set for the editor window.
		void SetWindowFlags(ImGuiWindowFlags flags) { m_windowFlags = flags; }

	private:
		bool m_hasToolbar = true;
		std::string m_name = "Window";
		ImGuiWindowFlags m_windowFlags = ImGuiWindowFlags_MenuBar;

		/// @brie  f Opens the editor window. This method is called before rendering the window.
		void OpenWindow();

		/// @brief Closes the editor window. This method is called after rendering the window.
		void CloseWindow();
	};
}