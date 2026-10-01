#pragma once
#include <string>

namespace Droplet::Editor::Scene
{
	/// @brief Opens a component list popup window that allows the user to select a component from the registered components.
	/// 
	/// This function opens the popup, but does not render it. You must call ShowComponentList() in your render loop to 
	/// actually show the popup and allow the user to select a component.
	void OpenComponentList();

	/// @brief Shows a component list popup window that allows the user to select a component from the registered components.
	/// @param p_outName A pointer to a string that will be filled with the name of the selected component if the user selects one.
	/// @return True if the user has selected a component, false otherwise.
	bool ShowComponentList(std::string *p_outName);
}
