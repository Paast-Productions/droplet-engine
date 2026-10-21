#pragma once
#include <Transform.hpp>

namespace Droplet::Editor
{
	/// @brief Shows the transform UI for a given transform component.
	/// @param p_transform Pointer to the transform component to display in the UI.
	/// @return True if the transform was edited, false otherwise.
	/// @throws std::invalid_argument if p_transform is null.
	bool ShowTransformUI(Droplet::Scene::Transform *p_transform);
}