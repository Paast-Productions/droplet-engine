#pragma once

namespace ImGui
{
	/// @brief Locks the mouse to the current item. 
	/// Useful for avoiding dragging interactions to be stopped by the mouse hitting the edge of the screen.
	/// @param p_hideMouse Whether to hide the mouse cursor while locked. Defaults to false.
	void LockToItem(bool p_hideMouse = false);
}