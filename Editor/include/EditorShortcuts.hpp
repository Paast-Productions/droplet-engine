#pragma once

namespace Droplet::Editor
{
	/// @brief Editor shortcuts is a quality of life class that is responsible for making the editor more efficient to use.
	/// Introducing keybinds that affect what you change in certain windows.
	class EditorShortcuts
	{
	public:
		EditorShortcuts() = default;
		~EditorShortcuts() = default;

		static void ProcessSceneViewKeyShortcuts();
	};
}
