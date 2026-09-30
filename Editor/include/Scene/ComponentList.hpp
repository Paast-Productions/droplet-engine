#pragma once
#include <Component.hpp>
#include <string>

namespace Droplet::Editor::Scene
{
	/// @brief Opens a component list popup window that allows the user to select a component from the registered components.
	/// @param p_outName A pointer to a string that will be filled with the name of the selected component if the user selects one.
	/// @return True if the user has selected a component, false otherwise.
	bool OpenComponentList(std::string *p_outName);
}
