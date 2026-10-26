#pragma once
#include <SceneSystem/Node.hpp>
namespace Droplet::Editor
{

	class EditorContext;
	/// @brief Editor shortcuts is a quality of life class that is responsible for making the editor more efficient to use.
	/// Introducing keybinds that affect what you change in certain windows.
	class EditorShortcuts
	{
	public:
		
		static EditorShortcuts &GetInstance()
		{
			static EditorShortcuts instance;
			return instance;
		}
		~EditorShortcuts() = default;

		/// @brief Processes key shortcuts for the scene view window.
		void ProcessSceneViewKeyShortcuts(EditorContext &p_editorContext);

	private:
		EditorShortcuts() = default;

		EditorShortcuts(const EditorShortcuts &) = delete;
		EditorShortcuts &operator=(const EditorShortcuts &) = delete;


		// HACK Make into a imgui clipboard instead HACK
		std::vector<std::weak_ptr<Droplet::Scene::Node>> m_copiedNodes = {};
	};
}
