#pragma once
#include <ImGui/imgui.h>
#include <string>

namespace ImGui
{
	/// @brief Locks the mouse to the current item. 
	/// Useful for avoiding dragging interactions to be stopped by the mouse hitting the edge of the screen.
	/// @param p_hideMouse Whether to hide the mouse cursor while locked. Defaults to false.
	void LockToItem(bool p_hideMouse = false);

	void SearchBar(std::string &p_outFilter, ImVec2 p_minMax = ImVec2(0.0f, 0.0f), bool p_focusOnAppearing = false);
}